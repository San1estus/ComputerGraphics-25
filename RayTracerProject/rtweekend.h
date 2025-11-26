#ifndef RTWEEKEND_H
#define RTWEEKEND_H

#include <cmath>
#include <random>
#include <iostream>
#include <limits>
#include <memory>


using std::make_shared;
using std::shared_ptr;

// Constantes

const double infinity = std::numeric_limits<double>::infinity();
const double pi = acos(-1);

// Funciones comunes

inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

inline double random_double() {
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
}

inline double random_double(double min, double max){
	return min + (max-min)*random_double();
}

// Headers comunes

#include "color.h"
#include "interval.h"
#include "ray.h"
#include "vec3.h"
#include "mat4.h"
#include "affine.h"

#endif