#include "player.h"

#include "rhizome/player_input.h"
#include "rhizome/sprite.h"
#include "rhizome/transform.h"
#include "camera.h"
#include "ship.h"

typedef struct {
} player_s;

component_h add_player_component(game_context_s *context, component_h parent)
{
    context = game_add_component(context, parent, release_component);

    component_h self = game_get_self(context);
    player_s *player = malloc(sizeof(player_s));
    game_set_component_data(context, player);

    transform_h transform = add_transform_component(context, self,
        vect_zero, quaternion_identity);

    component_h sprite = add_sprite_component(context, self);
    send_sprite_track_transform(context, sprite, transform);

    player_input_h player_input = add_player_input_component(context, self);

    ship_h ship = add_ship_component(context, self);
    ship_args_s ship_args = {transform, player_input};
    send_ship_init(context, handle_get(ship)->component, ship_args);

    broadcast_camera_follow_transform(context, transform);

    return self;
}

static void release_component(void *data)
{
    free(data);
}
