#include "dummy_scene.h"

#include "rhizome/group.h"
#include "rhizome/input_handler.h"
#include "rhizome/player_input.h"
#include "rhizome/renderer.h"
#include "rhizome/sprite.h"
#include "rhizome/transform.h"
#include "rhizome/vect.h"

begin_component(dummy_scene);
    component_subscribe(tick);
end_component();

typedef struct
{
    component_h group;
    transform_h transform;
    player_input_h player_input;
} dummy_scene_s;

static component_h init(game_context_s *context)
{
    component_h self = game_get_self(context);
    dummy_scene_s *dummy_scene = malloc(sizeof(dummy_scene_s));
    game_set_component_data(context, dummy_scene);

    add_input_handler_component(context, self);
    add_renderer_component(context, self);

    dummy_scene->group = add_group_component(context, self);
    dummy_scene->transform =
        add_transform_component(context, dummy_scene->group);
    send_transform_set_pos(
        context,
        handle_get(dummy_scene->transform)->component,
        make_vect(50, 50, 0));
    dummy_scene->player_input =
        add_player_input_component(context, dummy_scene->group);

    component_h sprite = add_sprite_component(context, dummy_scene->group);
    send_sprite_track_transform(context, sprite, dummy_scene->transform);

    return self;
}

static void release(void *data)
{
    free(data);
}

static void handle_tick(game_context_s *context, void *data, const void **dummy)
{
    dummy_scene_s *dummy_scene = data;
    const transform_s *transform = handle_get(dummy_scene->transform);
    const player_input_s *player_input = handle_get(dummy_scene->player_input);
    if(transform && player_input)
    {
        send_transform_move(context, transform->component,
            player_input->direction);
        send_transform_rotate(context, transform->component,
            make_quaternion_rotation(
                vect_normalize(make_vect(1, 1, 1)),
                0.01));

        if(transform->pos.x > 100)
            game_remove_component(context, dummy_scene->group);
    }
}
