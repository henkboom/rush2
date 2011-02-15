#ifndef NOISE_H
#define NOISE_H

#include "rhizome/vect.h"

double noise_generator_sample_at(vect_s pos);
vect_s noise_generator_normal_at(vect_s pos);

#endif
