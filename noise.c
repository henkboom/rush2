#include "noise.h"

#include <math.h>

double noise_generator_sample_at(vect_s pos)
{
    return sin(pos.x/6.0) + sin(pos.z/7.0)/2 + sin(pos.x/30 + pos.z/37);
}

vect_s noise_generator_normal_at(vect_s pos)
{
    double at_point = noise_generator_sample_at(pos);
    double dx = noise_generator_sample_at(vect_add(pos, make_vect(0.1, 0, 0))) -
        at_point;
    double dz = noise_generator_sample_at(vect_add(pos, make_vect(0, 0, 0.1))) -
        at_point;
    vect_s normal = vect_cross(make_vect(0, dz, 0.1), make_vect(0.1, dx, 0));
    return normal;
}
