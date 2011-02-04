#include "terrain_renderer.h"

#include <GL/gl.h>

#include "rhizome/renderer.h"

begin_component(terrain_renderer);
end_component();

typedef struct {
    render_job_s render_job;
} terrain_renderer_s;

static void render(const render_context_s *context, const render_job_s *data);

static component_h init(game_context_s *context)
{
    component_h self = game_get_self(context);
    terrain_renderer_s *terrain_renderer = malloc(sizeof(terrain_renderer_s));
    game_set_component_data(context, terrain_renderer);

    terrain_renderer->render_job.render = render;

    render_job_h handle;
    game_add_buffer(context, terrain_renderer, sizeof(terrain_renderer_s),
                    (void *)&handle);
    broadcast_renderer_add_job(context, handle);

    return self;
}

static void release(void *data)
{
}

static void render(const render_context_s *context, const render_job_s *data)
{
    glBegin(GL_LINES);
    for(int i = -100; i <= 100; i++)
    {
        glVertex3d(i, 0, -100);
        glVertex3d(i, 0, 100);
        glVertex3d(-100, 0, i);
        glVertex3d(100, 0, i);
    }
    glEnd();
}
