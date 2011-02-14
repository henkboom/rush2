#ifndef CAMERA_H
#define CAMERA_H

#include "rhizome/game.h"
#include "rhizome/transform.h"

component_h add_camera_component(game_context_s *context, component_h parent);

define_broadcast(camera_follow_transform, transform_h);

#endif
