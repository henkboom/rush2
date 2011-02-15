#include "terrain_renderer.h"

#include <math.h>
#include <GL/glew.h>

#include "rhizome/array.h"
#include "rhizome/renderer.h"
#include "graphics.h"

#define BLOCK_SIZE 128
#define TRIANGLE_SIZE 1.0

typedef struct {
    render_job_s render_job;
    GLuint program;
    GLuint vertex_buffer;
    GLuint vertex_type_buffer;
    GLuint element_buffer;
} terrain_renderer_job_s;
define_handle_type(terrain_renderer_job_h, terrain_renderer_job_s);

typedef struct {
    // exclusively accessed by 'renderer' during 'tick'
    terrain_renderer_job_h render_job;
} terrain_renderer_s;

static void render(const render_context_s *context, const render_job_s *data);

component_h add_terrain_renderer_component(
    game_context_s *context,
    component_h parent)
{
    context = game_add_component(context, parent, release_component);

    component_h self = game_get_self(context);
    terrain_renderer_s *terrain_renderer = malloc(sizeof(terrain_renderer_s));
    game_set_component_data(context, terrain_renderer);

    terrain_renderer_job_s *job = malloc(sizeof(terrain_renderer_job_s));
    handle_new(&terrain_renderer->render_job, job);

    job->render_job.render = render;
    job->program = 0;
    job->vertex_buffer = 0;

    broadcast_renderer_add_job(
        context,
        *(render_job_h*)&terrain_renderer->render_job);

    return self;
}

static void release_component(void *data)
{
    terrain_renderer_s *terrain_renderer = data;
    terrain_renderer_job_s *job = handle_get(terrain_renderer->render_job);

    free(job);
    handle_release(terrain_renderer->render_job);

    free(terrain_renderer);
}

#define SQRT3 1.7320508075689
static inline vect_s pos_at(int i, int j)
{
    return make_vect(
        (i + j * 0.5) * TRIANGLE_SIZE,
        sin(i/6.0)/2 + sin(j/7.0)/2 - 1,
        j * SQRT3/2 * TRIANGLE_SIZE);
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
        // degenerate triangles
        array_add(elements, (i  ));
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

static void render(const render_context_s *context, const render_job_s *data)
{
    terrain_renderer_job_s *job = (terrain_renderer_job_s *)data;

    if(job->program == 0)
    {
        GLuint shaders[2];
        shaders[0] = graphics_create_shader_from_file(
            GL_VERTEX_SHADER, "terrain.v.glsl");
        shaders[1] = graphics_create_shader_from_file(
            GL_FRAGMENT_SHADER, "terrain.f.glsl");
        job->program = graphics_create_program(2, shaders);
    }

    if(job->vertex_buffer == 0)
    {
        create_buffers(
            &job->vertex_buffer,
            &job->vertex_type_buffer,
            &job->element_buffer);
    }

    glUseProgram(job->program);

    glBindBuffer(GL_ARRAY_BUFFER, job->vertex_buffer);
    glVertexAttribPointer(glGetAttribLocation(job->program, "vertex"),
        3, GL_DOUBLE, GL_FALSE, sizeof(double) * 3, (void *)0);
    glEnableVertexAttribArray(glGetAttribLocation(job->program, "vertex"));

    glBindBuffer(GL_ARRAY_BUFFER, job->vertex_type_buffer);
    glVertexAttribPointer(glGetAttribLocation(job->program, "vertex_type"),
        1, GL_FLOAT, GL_FALSE, sizeof(float), (void *)0);
    glEnableVertexAttribArray(
        glGetAttribLocation(job->program, "vertex_type"));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, job->element_buffer);
    glDrawElements(GL_TRIANGLE_STRIP, BLOCK_SIZE*2*(BLOCK_SIZE-1),
            GL_UNSIGNED_INT, (void *)0);

    glUseProgram(0);
}
