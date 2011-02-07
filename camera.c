#include "camera.h"

#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "rhizome/renderer.h"

begin_component(camera);
    component_subscribe(tick);
    component_subscribe(camera_follow_transform);
end_component();

typedef struct {
    render_job_s render_job;
    transform_h transform;
    vect_s aim;
    vect_s vel;
} camera_s;

static void render(const render_context_s *context, const render_job_s *data);

static component_h init(game_context_s *context)
{
    component_h self = game_get_self(context);
    camera_s *camera = malloc(sizeof(camera_s));
    game_set_component_data(context, camera);

    camera->render_job.render = render;
    camera->transform = null_handle(transform_h);
    camera->aim = make_vect(0, 0, 0);
    camera->vel = make_vect(0, 0, 0);

    render_job_h handle;
    game_add_buffer(context, camera, sizeof(camera_s), (void_h *)&handle);
    broadcast_renderer_add_job(context, handle);

    return self;
}

static void release(void *data)
{
    free(data);
}

static void handle_tick(game_context_s *context, void *data, const nothing_s *n)
{
    camera_s *camera = data;

    const transform_s *transform = handle_get(camera->transform);
    if(transform)
    {
        vect_s new_aim = transform->pos;
        camera->vel = vect_add(
            vect_mul(camera->vel, 0.93),
            vect_mul(vect_sub(new_aim, camera->aim), 0.07));
        camera->aim = new_aim;
    }
}

static void handle_camera_follow_transform(
    game_context_s *context,
    void *data,
    const transform_h *transform)
{
    camera_s *camera = data;
    camera->transform = *transform;
    if(handle_get(*transform))
        camera->aim = handle_get(*transform)->pos;
}

static void render(const render_context_s *context, const render_job_s *data)
{
    const camera_s *camera = (camera_s *)data;

    glMatrixMode(GL_PROJECTION);
    gluPerspective(65, (double)context->width/context->height, 0.1, 100);
    glMatrixMode(GL_MODELVIEW);

    vect_s up = make_vect(1, 0, 0);
    if(vect_sqrmag(camera->vel) > 0)
        up = camera->vel;

    vect_s source = vect_sub(camera->aim, vect_mul(camera->vel, 3));
    vect_s target = vect_add(camera->aim, vect_mul(camera->vel, 6));
    source.y += fmax(10 - vect_magnitude(camera->vel) * 10, 1);
    
    gluLookAt(
        source.x, source.y, source.z,
        target.x, target.y, target.z,
        up.x, up.y, up.z);
}
