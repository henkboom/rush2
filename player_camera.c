#include "player_camera.h"

#include <math.h>
#include <GL/glfw.h>

typedef struct {
    transform_h target;
    camera_h camera;
    vect_s aim;
    vect_s vel;
} player_camera_s;

component_h add_player_camera_component(
    game_context_s *context,
    component_h parent,
    camera_h camera,
    transform_h target)
{
    context = game_add_component(context, parent, release_component);

    component_subscribe(context, tick);

    component_h self = game_get_self(context);
    player_camera_s *player_camera = malloc(sizeof(player_camera_s));
    game_set_component_data(context, player_camera);

    player_camera->target = target;
    player_camera->camera = camera;
    player_camera->aim = make_vect(0, 0, 0);
    player_camera->vel = make_vect(0, 0, 0);

    return self;
}

static void release_component(void *data)
{
    free(data);
}

static void handle_tick(game_context_s *context, void *data, const nothing_s *n)
{
    player_camera_s *player_camera = data;

    const transform_s *target = handle_get(player_camera->target);
    if(target)
    {
        vect_s forward = quaternion_rotate_i(target->orientation);
        vect_s new_aim = target->pos;
        player_camera->vel = vect_add(
            vect_mul(player_camera->vel, 0.93),
            vect_mul(vect_sub(new_aim, player_camera->aim), 0.07));
        player_camera->aim = new_aim;

        vect_s up = make_vect(1, 0, 0);
        if(vect_sqrmag(player_camera->vel) > 0)
            up = vect_add(player_camera->vel, forward);

        vect_s source_pos =
            vect_sub(new_aim, vect_mul(vect_add(player_camera->vel, forward), 3));
        vect_s target_pos =
            vect_add(new_aim, vect_mul(vect_add(player_camera->vel, forward), 6));
        //vect_s source_pos = make_vect(-1, 5, -1);
        //vect_s target_pos = make_vect(10, 0, 10);
        source_pos.y += fmax(10 - vect_magnitude(player_camera->vel) * 10, 1);

        const camera_s *camera = handle_get(player_camera->camera);
        const transform_s *camera_transform = handle_get(camera->transform);

        send_transform_set_pos(context, camera_transform->component,
            source_pos);
        send_transform_set_orientation(context, camera_transform->component,
            make_look_quaternion(vect_sub(target_pos, source_pos), up));
    }
}
