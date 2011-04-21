// TODO remove
#define _POSIX_C_SOURCE 2
#include <signal.h>
#include <stdio.h>

#include "ship.h"


#include <math.h>

#include "rhizome/quaternion.h"
#include "rhizome/vect.h"
#include "noise.h"
//TODO remove
#include "kinect.h"

//TODO remove!!
/*global*/ double player_speed;

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

static double height_at(vect_s pos)
{
    return pos.y - noise_generator_sample_at(pos) - 0.2;
}

static int past_collision_height(vect_s pos)
{
    return height_at(pos) < -2;
}

static void handle_tick(game_context_s *context, void *data, const nothing_s *n)
{
    ship_s *ship = data;
    const transform_s *transform = handle_get(ship->transform);
    vect_s forward = quaternion_rotate_i(transform->orientation);

    //// Controls
    vect_s control = handle_get(ship->player_input)->direction;
    double slope = kinect_get_slope();
    //printf("%lf\n", slope);
    if(slope > -1 && slope < 1)
    {
        control.x -= slope;
        control.y += fmax(1-2*fabs(slope), 0);
    }

    //// Acceleration
    ship->velocity = vect_add(ship->velocity, vect_mul(forward, control.y/100));
    ship->velocity = vect_add(ship->velocity, make_vect(0, -1.0/30, 0));

    //// Velocity
    vect_s sideways_vel = vect_project(
        ship->velocity,
        quaternion_rotate_k(transform->orientation));
    ship->velocity = vect_sub(
        ship->velocity,
        vect_mul(sideways_vel, 0.3));
    ship->velocity = vect_mul(ship->velocity, 0.995);
    vect_s movement = ship->velocity;

    //// Wall Collision Detection
    if(past_collision_height(vect_add(movement, transform->pos)))
    {
        vect_s contact_normal = vect_zero;
        if(past_collision_height(
            vect_add(transform->pos, make_vect(movement.x, movement.y, 0))))
        {
            contact_normal = movement.x > 0 ? vect_neg(vect_i) : vect_i;
        }
        else
        {
            contact_normal = movement.z > 0 ? vect_neg(vect_k) : vect_k;
        }
        ship->velocity = vect_sub(ship->velocity,
            vect_mul(vect_project(ship->velocity, contact_normal), 1.5));
        movement = vect_sub(movement,
            vect_mul(vect_project(movement, contact_normal), 1.5));
    }

    //// Ground Collision Detection
    double height = height_at(vect_add(movement, transform->pos));
    if(height <= 0)
    {
        vect_s normal = noise_generator_normal_at(transform->pos);
        //print_vect(ship->velocity);
        //print_vect(normal);
        if(vect_dot(normal, ship->velocity) < 0)
        {
            ship->velocity = vect_sub(
                ship->velocity,
                vect_div(vect_project(ship->velocity, normal), 2));
        }
    }
    if(height < 0)
        movement = vect_add(movement, make_vect(0, -height, 0));

    //// Rotation
    quaternion_s rotation = make_quaternion_rotation(
        make_vect(0, 1, 0),
        3.14159/64 * -control.x);

    //// Send Transform Messages
    send_transform_move(context, transform->component, movement);
    send_transform_rotate(context, transform->component, rotation);

    //// Send Speed to PureData
    // TODO move this elsewhere
    player_speed = vect_magnitude(ship->velocity);
    static FILE *out = NULL;
    signal(SIGPIPE, SIG_IGN);
    if(out == NULL)
        out = popen("pdsend 3000", "w");
    if(out != NULL)
    {
        if(fprintf(out, "%lf;\n", player_speed) >= 0)
            fflush(out);
        else
        {
            pclose(out);
            out = NULL;
        }
    }
}
