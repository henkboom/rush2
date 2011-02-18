#ifndef PLAYER_CAMERA_H
#define PLAYER_CAMERA_H

#include "rhizome/camera.h"
#include "rhizome/game.h"
#include "rhizome/transform.h"

#include "player.h"

component_h add_player_camera_component(
    game_context_s *context,
    component_h parent,
    camera_h camera,
    transform_h target);

#endif
