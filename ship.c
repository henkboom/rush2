#include "ship.h"

#include <math.h>

#include "rhizome/quaternion.h"
#include "rhizome/vect.h"

begin_component(ship);
    component_subscribe(ship_init);
    component_subscribe(tick);
end_component();

static ship_h init(game_context_s *context)
{
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

static void release(void *data)
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

    ship->velocity = vect_add(ship->velocity,
        vect_mul(quaternion_rotate_i(transform->orientation), control.y/60));
    // damp
    ship->velocity = vect_mul(ship->velocity, 0.98);

    send_transform_rotate(context, transform->component, rotation);
    send_transform_move(context, transform->component, ship->velocity);
}
