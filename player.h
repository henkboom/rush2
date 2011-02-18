#ifndef PLAYER_H
#define PLAYER_H

#include "rhizome/game.h"
#include "rhizome/transform.h"

typedef struct {
    component_h component;
    transform_h transform;
} player_s;
define_handle_type(player_h, const player_s);

player_h add_player_component(
    game_context_s *context,
    component_h parent);

#endif
