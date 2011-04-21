#include "noise.h"

#include <math.h>

static double waves(vect_s pos, double in)
{
    return sin(pos.x/6.0)/1.5 + sin(pos.z/7.0)/2 + sin(pos.x/30 + pos.z/37)*3
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
    const double separation = 500;
    const double width = 15;
    double tx = mod_difference(pos.x, separation);
    double ty = mod_difference(pos.z, separation);
    double t = fmin(tx, ty);
    double fade = fmin(1, fmax(0, (40 - (tx-width/2) - (ty-width/2)) / 30.0));

    double road_height = sin((pos.x + pos.z)*3.14159/800)*15;
    if(t < width/2)
        return road_height;
    else
        return (1-fade)*in + fade*road_height;
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
    double dx1 = noise_generator_sample_at(vect_add(pos, make_vect(0.1, 0, 0))) - at_point;
    double dx2 = at_point - noise_generator_sample_at(vect_add(pos, make_vect(-0.1, 0, 0)));
    double dz1 = noise_generator_sample_at(vect_add(pos, make_vect(0, 0, 0.1))) - at_point;
    double dz2 = at_point - noise_generator_sample_at(vect_add(pos, make_vect(0, 0, -0.1)));
    //printf("two normals: ");
    //print_vect(vect_cross(make_vect(0, dz1, 0.1), make_vect(0.1, dx1, 0)))
    //print_vect(vect_cross(make_vect(0, dz1, 0.1), make_vect(0.1, dx1, 0)))
    vect_s normal = vect_normalize(vect_add(
        vect_normalize(vect_cross(make_vect(0, dz1, 0.1), make_vect(0.1, dx1, 0))),
        vect_normalize(vect_cross(make_vect(0, dz2, 0.1), make_vect(0.1, dx2, 0)))));
    return normal;
}
