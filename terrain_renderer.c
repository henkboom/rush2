#include "terrain_renderer.h"

#include <math.h>
#include <GL/glew.h>

#include "rhizome/array.h"
#include "rhizome/graphics.h"
#include "rhizome/renderer.h"
#include "noise.h"

// number of vertices along the edge of the block
#define TERRAIN_BLOCK_VERTICES 31
#define TERRAIN_TRIANGLE_SIZE \
    ((double)TERRAIN_BLOCK_WIDTH/(TERRAIN_BLOCK_VERTICES-1))

typedef struct {
    vect_s offset;
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
    component_h parent,
    vect_s offset)
{
    context = game_add_component(context, parent, release_component);

    component_h self = game_get_self(context);
    terrain_renderer_s *terrain_renderer = malloc(sizeof(terrain_renderer_s));
    game_set_component_data(context, terrain_renderer);

    terrain_renderer->render_job.priority = 1;
    terrain_renderer->render_job.render = render;
    terrain_renderer->render_job.data = &terrain_renderer->render_data;

    terrain_renderer->render_data.offset = offset;
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
    terrain_renderer_s *terrain_renderer = data;
    glDeleteBuffers(1, &terrain_renderer->render_data.vertex_buffer);
    glDeleteBuffers(1, &terrain_renderer->render_data.vertex_type_buffer);
    glDeleteBuffers(1, &terrain_renderer->render_data.element_buffer);
    free(terrain_renderer);
}

#define SQRT3 1.7320508075689
static inline vect_s pos_at(vect_s offset, int i, int j)
{
    vect_s ret = make_vect(
        offset.x + (i + j * 0.5) * TERRAIN_TRIANGLE_SIZE,
        0,
        offset.z + j * SQRT3/2 * TERRAIN_TRIANGLE_SIZE);
    ret.y = noise_generator_sample_at(ret);
    return ret;
}

static void create_buffers(
    vect_s offset,
    GLuint *vertex_buffer,
    GLuint *vertex_type_buffer,
    GLuint *element_buffer)
{
    array_of(double) vertices = array_new();
    array_of(float) vertex_types = array_new();
    array_of(unsigned) elements = array_new();

    for(int j = 0; j < TERRAIN_BLOCK_VERTICES; j++)
    {
        for(int i = 0; i < TERRAIN_BLOCK_VERTICES; i++)
        {
            vect_s pos = pos_at(offset, i, j);
            array_add(vertices, pos.x);
            array_add(vertices, pos.y);
            array_add(vertices, pos.z);
            array_add(vertex_types, ((i+j*2)%3) ? 1 : 0);
        }
    }

    for(int i = 0; i < TERRAIN_BLOCK_VERTICES-1; i++)
    {
        for(int j = TERRAIN_BLOCK_VERTICES-1; j >= 0; j--)
        {
            array_add(elements, i+1+j*TERRAIN_BLOCK_VERTICES);
            array_add(elements, i+j*TERRAIN_BLOCK_VERTICES);
        }
        // degenerate triangles to switch to the next strip
        array_add(elements, i);
        array_add(elements, i+2+(TERRAIN_BLOCK_VERTICES-1)*TERRAIN_BLOCK_VERTICES);
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

// TODO this should be done in some sort of resource manager
static GLuint program = 0;

static void render(const render_context_s *context, void *data)
{
    render_data_s *render_data = data;

    if(program != 0)
        render_data->program = program;
    if(render_data->program == 0)
    {
        GLuint shaders[2];
        shaders[0] = graphics_create_shader_from_file(
            GL_VERTEX_SHADER, "terrain.v.glsl");
        shaders[1] = graphics_create_shader_from_file(
            GL_FRAGMENT_SHADER, "terrain.f.glsl");
        render_data->program = graphics_create_program(2, shaders);
    }
    if(program == 0)
        program = render_data->program;

    if(render_data->vertex_buffer == 0)
    {
        create_buffers(
            render_data->offset,
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
    glDrawElements(GL_TRIANGLE_STRIP,
            (TERRAIN_BLOCK_VERTICES*2+2)*(TERRAIN_BLOCK_VERTICES-1),
            GL_UNSIGNED_INT, (void *)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(
        glGetAttribLocation(render_data->program, "vertex"));
    glDisableVertexAttribArray(
        glGetAttribLocation(render_data->program, "vertex_type"));

    glUseProgram(0);
}
