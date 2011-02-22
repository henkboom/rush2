#include "player.h"

#include "rhizome/player_input.h"
#include "rhizome/sprite.h"
#include "rhizome/transform.h"
#include "ship.h"

player_h add_player_component(game_context_s *context, component_h parent)
{
    context = game_add_component(context, parent, release_component);

    component_h self = game_get_self(context);
    player_s *player = malloc(sizeof(player_s));
    game_set_component_data(context, player);

    player->component = self;
    player->transform = add_transform_component(context, self,
        vect_zero, quaternion_identity);

    add_sprite_component(context, self, player->transform, NULL);

    player_input_h player_input = add_player_input_component(context, self);

    ship_h ship = add_ship_component(context, self);
    ship_args_s ship_args = {player->transform, player_input};
    send_ship_init(context, handle_get(ship)->component, ship_args);

    player_h handle;
    game_add_buffer(context,player, sizeof(player_s), (void_h*)&handle);
    return handle;
}

static void release_component(void *data)
{
    free(data);
}
