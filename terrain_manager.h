#ifndef TERRAIN_MANAGER_H
#define TERRAIN_MANAGER_H

#include "rhizome/game.h"
#include "rhizome/transform.h"

component_h add_terrain_manager_component(
    game_context_s *context,
    component_h parent,
    transform_h center);

#endif
