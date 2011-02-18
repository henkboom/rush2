#include "terrain_renderer.h"

#include <math.h>
#include <GL/glew.h>

#include "rhizome/array.h"
#include "rhizome/renderer.h"
#include "graphics.h"
#include "noise.h"

#define BLOCK_SIZE 128
#define TRIANGLE_SIZE 1.0

typedef struct {
    GLuint program;
    GLuint vertex_buffer;
    GLuint vertex_type_buffer;
    GLuint element_buffer;
} render_data_s;

typedef struct {
    render_job_s render_job;
    // exclusively accessed by 'renderer' during 'tick'
    render_data_s render_data;
} terrain_renderer_s;

static void render(const render_context_s *context, void *data);

component_h add_terrain_renderer_component(
    game_context_s *context,
    component_h parent)
{
    context = game_add_component(context, parent, release_component);

    component_h self = game_get_self(context);
    terrain_renderer_s *terrain_renderer = malloc(sizeof(terrain_renderer_s));
    game_set_component_data(context, terrain_renderer);

    terrain_renderer->render_job.priority = 1;
    terrain_renderer->render_job.render = render;
    terrain_renderer->render_job.data = &terrain_renderer->render_data;

    terrain_renderer->render_data.program = 0;
    terrain_renderer->render_data.vertex_buffer = 0;
    terrain_renderer->render_data.vertex_type_buffer = 0;
    terrain_renderer->render_data.element_buffer = 0;

    render_job_h job;
    game_add_buffer(context, &terrain_renderer->render_job,
        sizeof(render_job_s), (void_h*)&job);
    broadcast_renderer_add_job(context, job);

    return self;
}

static void release_component(void *data)
{
    free(data);
}

#define SQRT3 1.7320508075689
static inline vect_s pos_at(int i, int j)
{
    vect_s ret = make_vect(
        (i + j * 0.5) * TRIANGLE_SIZE,
        0,
        j * SQRT3/2 * TRIANGLE_SIZE);
    ret.y = noise_generator_sample_at(ret);
    return ret;
}

static void create_buffers(
    GLuint *vertex_buffer,
    GLuint *vertex_type_buffer,
    GLuint *element_buffer)
{
    array_of(double) vertices = array_new();
    array_of(float) vertex_types = array_new();
    array_of(unsigned) elements = array_new();

    for(int j = 0; j < BLOCK_SIZE; j++)
    {
        for(int i = 0; i < BLOCK_SIZE; i++)
        {
            vect_s pos = pos_at(i, j);
            array_add(vertices, pos.x);
            array_add(vertices, pos.y);
            array_add(vertices, pos.z);
            array_add(vertex_types, ((i+j*2)%3) ? 1 : 0);
        }
    }

    for(int i = 0; i < BLOCK_SIZE-1; i++)
    {
        for(int j = BLOCK_SIZE-1; j >= 0; j--)
        {
            array_add(elements, i+1+j*BLOCK_SIZE);
            array_add(elements, i+j*BLOCK_SIZE);
        }
        // degenerate triangles to switch to the next strip
        array_add(elements, i);
        array_add(elements, i+2+(BLOCK_SIZE-1)*BLOCK_SIZE);
    }

    *vertex_buffer = graphics_make_buffer(
        GL_ARRAY_BUFFER,
        array_length(vertices) * array_element_size(vertices),
        array_get_ptr(vertices),
        GL_STATIC_DRAW);

    *vertex_type_buffer = graphics_make_buffer(
        GL_ARRAY_BUFFER,
        array_length(vertex_types) * array_element_size(vertex_types),
        array_get_ptr(vertex_types),
        GL_STATIC_DRAW);

    *element_buffer = graphics_make_buffer(
        GL_ELEMENT_ARRAY_BUFFER,
        array_length(elements) * array_element_size(elements),
        array_get_ptr(elements),
        GL_STATIC_DRAW);

    array_release(vertices);
    array_release(vertex_types);
    array_release(elements);
}

static void render(const render_context_s *context, void *data)
{
    render_data_s *render_data = data;

    if(render_data->program == 0)
    {
        GLuint shaders[2];
        shaders[0] = graphics_create_shader_from_file(
            GL_VERTEX_SHADER, "terrain.v.glsl");
        shaders[1] = graphics_create_shader_from_file(
            GL_FRAGMENT_SHADER, "terrain.f.glsl");
        render_data->program = graphics_create_program(2, shaders);
    }

    if(render_data->vertex_buffer == 0)
    {
        create_buffers(
            &render_data->vertex_buffer,
            &render_data->vertex_type_buffer,
            &render_data->element_buffer);
    }

    glUseProgram(render_data->program);

    glBindBuffer(GL_ARRAY_BUFFER, render_data->vertex_buffer);
    glVertexAttribPointer(glGetAttribLocation(render_data->program, "vertex"),
        3, GL_DOUBLE, GL_FALSE, sizeof(double) * 3, (void *)0);
    glEnableVertexAttribArray(
        glGetAttribLocation(render_data->program, "vertex"));

    glBindBuffer(GL_ARRAY_BUFFER, render_data->vertex_type_buffer);
    glVertexAttribPointer(
        glGetAttribLocation(render_data->program, "vertex_type"),
        1, GL_FLOAT, GL_FALSE, sizeof(float), (void *)0);
    glEnableVertexAttribArray(
        glGetAttribLocation(render_data->program, "vertex_type"));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_data->element_buffer);
    glDrawElements(GL_TRIANGLE_STRIP, BLOCK_SIZE*2*(BLOCK_SIZE-1),
            GL_UNSIGNED_INT, (void *)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(
        glGetAttribLocation(render_data->program, "vertex"));
    glDisableVertexAttribArray(
        glGetAttribLocation(render_data->program, "vertex_type"));

    glUseProgram(0);
}
