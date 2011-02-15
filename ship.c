#include "ship.h"

#include <math.h>

#include "rhizome/quaternion.h"
#include "rhizome/vect.h"
#include "noise.h"

ship_h add_ship_component(game_context_s *context, component_h parent)
{
    context = game_add_component(context, parent, release_component);

    component_subscribe(context, ship_init);
    component_subscribe(context, tick);

    component_h self = game_get_self(context);
    ship_s *ship = malloc(sizeof(ship_s));
    game_set_component_data(context, ship);

    ship->component = self;
    ship->transform = null_handle(transform_h);
    ship->player_input = null_handle(player_input_h);
    ship->velocity = make_vect(0, 0, 0);

    ship_h handle;
    game_add_buffer(context, ship, sizeof(ship_s), (void_h *)&handle);
    return handle;
}

static void release_component(void *data)
{
    free(data);
}

static void handle_ship_init(
    game_context_s *context,
    void *data,
    const ship_args_s *args)
{
    ship_s *ship = data;
    ship->transform = args->transform;
    ship->player_input = args->player_input;
}

static void handle_tick(game_context_s *context, void *data, const nothing_s *n)
{
    ship_s *ship = data;

    vect_s control = handle_get(ship->player_input)->direction;

    quaternion_s rotation = make_quaternion_rotation(
        make_vect(0, 1, 0),
        3.14159/128 * -control.x);

    const transform_s *transform = handle_get(ship->transform);

    // in ground?
    double height =
        transform->pos.y - noise_generator_sample_at(transform->pos) - 0.2;
    if(height <= 0)
    {
        vect_s normal = noise_generator_normal_at(transform->pos);
        if(vect_dot(normal, ship->velocity) < 0)
        {
            ship->velocity = vect_sub(
                ship->velocity,
                vect_div(vect_project(ship->velocity, normal), 4));
        }
    }

    ship->velocity = vect_add(
        ship->velocity,
        vect_mul(quaternion_rotate_i(transform->orientation), control.y/50));
    ship->velocity = vect_add(ship->velocity, make_vect(0, -0.5/60, 0));
    // damp
    ship->velocity = vect_mul(ship->velocity, 0.98);

    send_transform_rotate(context, transform->component, rotation);

    vect_s movement = ship->velocity;
    if(height < 0)
        movement = vect_add(movement, make_vect(0, -height, 0));
    send_transform_move(context, transform->component, movement);
}
