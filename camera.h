#ifndef CAMERA_H
#define CAMERA_H

#include "rhizome/game.h"
#include "rhizome/transform.h"

declare_component(camera, component_h);

define_broadcast(camera_follow_transform, transform_h);

#endif
