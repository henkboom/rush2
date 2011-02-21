#ifndef TERRAIN_RENDERER_H
#define TERRAIN_RENDERER_H

#include "rhizome/game.h"
#include "rhizome/vect.h"

// Each block is a rhombus formed by two equilateral triangles
//    _____
//   /\   /
//  /  \ /
// /____/
//
// The triangle side lengths are all TERRAIN_BLOCK_WIDTH

#define TERRAIN_BLOCK_WIDTH 30

component_h add_terrain_renderer_component(
    game_context_s *context,
    component_h parent,
    vect_s offset);

#endif
