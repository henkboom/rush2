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
    mesh_s *mesh;
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
    terrain_renderer->render_data.mesh = NULL;

    render_job_h job;
    game_add_buffer(context, &terrain_renderer->render_job,
        sizeof(render_job_s), (void_h*)&job);
    broadcast_renderer_add_job(context, job);

    return self;
}

static void release_component(void *data)
{
    terrain_renderer_s *terrain_renderer = data;
    mesh_release(terrain_renderer->render_data.mesh);
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

static mesh_s * create_mesh(vect_s offset)
{
    // shader
    // TODO this should be cached in some sort of resource manager
    static GLuint program = 0;
    if(program == 0)
    {
        GLuint shaders[2];
        shaders[0] = graphics_create_shader_from_file(
                GL_VERTEX_SHADER, "terrain.v.glsl");
        shaders[1] = graphics_create_shader_from_file(
                GL_FRAGMENT_SHADER, "terrain.f.glsl");
        program = graphics_create_program(2, shaders);
    }

    // geometry
    array_of(float) positions = array_new();
    array_of(float) edge_states = array_new();
    array_of(unsigned) elements = array_new();

    for(int j = 0; j < TERRAIN_BLOCK_VERTICES; j++)
    {
        for(int i = 0; i < TERRAIN_BLOCK_VERTICES; i++)
        {
            vect_s pos = pos_at(offset, i, j);
            array_add(positions, pos.x);
            array_add(positions, pos.y);
            array_add(positions, pos.z);
            array_add(edge_states, ((i+j*2)%3) ? 1 : 0);
        }
    }

    for(int i = 0; i < TERRAIN_BLOCK_VERTICES-1; i++)
    {
        for(int j = TERRAIN_BLOCK_VERTICES-1; j > 0; j--)
        {
            array_add(elements, (i+1)+(j  )*TERRAIN_BLOCK_VERTICES);
            array_add(elements, (i  )+(j  )*TERRAIN_BLOCK_VERTICES);
            array_add(elements, (i+1)+(j-1)*TERRAIN_BLOCK_VERTICES);

            array_add(elements, (i+1)+(j-1)*TERRAIN_BLOCK_VERTICES);
            array_add(elements, (i  )+(j  )*TERRAIN_BLOCK_VERTICES);
            array_add(elements, (i  )+(j-1)*TERRAIN_BLOCK_VERTICES);
        }
    }

    mesh_s *mesh = mesh_new();
    mesh_set_program(mesh, program);
    mesh_set_elements(mesh,
        array_length(elements) * array_element_size(elements),
        array_get_ptr(elements));
    mesh_set_attribute(mesh, "position", 3,
        array_length(positions) * array_element_size(positions),
        array_get_ptr(positions));
    mesh_set_attribute(mesh, "on_edge", 1,
        array_length(edge_states) * array_element_size(edge_states),
        array_get_ptr(edge_states));

    array_release(positions);
    array_release(edge_states);
    array_release(elements);

    return mesh;
}


static void render(const render_context_s *context, void *data)
{
    render_data_s *render_data = data;

    if(render_data->mesh == 0)
        render_data->mesh = create_mesh(render_data->offset);

    mesh_render(render_data->mesh);
}
