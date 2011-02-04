#ifndef SHIP_H
#define SHIP_H

#include "rhizome/game.h"
#include "rhizome/transform.h"
#include "rhizome/player_input.h"

typedef struct {
    component_h component;
    transform_h transform;
    player_input_h player_input;
    vect_s velocity;
} ship_s;
define_handle_type(ship_h, const ship_s);

declare_component(ship, ship_h);

typedef struct {
    transform_h transform;
    player_input_h player_input;
} ship_args_s;

define_message(ship_init, ship_args_s);

#endif
