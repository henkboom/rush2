#include "noise.h"

#include <math.h>

static double waves(vect_s pos, double in)
{
    return sin(pos.x/6.0) + sin(pos.z/7.0)/2 + sin(pos.x/30 + pos.z/37)*3
         + sin(pos.x/61 - pos.z/87)*5;
}

static double mod_difference(double a, double b)
{
    double ret = fmod(a + b/2, b);
    if(ret < 0)
        ret += b;
    return fabs(ret - b/2);
}

static double roads(vect_s pos, double in)
{
    const double separation = 200;
    const double width = 10;
    double t = fmin(
        mod_difference(pos.x, separation),
        mod_difference(pos.z, separation));

    if(t < width/2)
        return sin((pos.x + pos.z)*3.14159/500)*10;
    else
        return in;
}

double noise_generator_sample_at(vect_s pos)
{
    pos.y = 0;
    double y = 0;
    y = waves(pos, y);
    y = roads(pos, y);
    return y;
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
