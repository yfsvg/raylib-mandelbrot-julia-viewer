#ifndef UI_TYPES_HPP
#define UI_TYPES_HPP

#include "raylib.h"
#include "raymath.h"

#include <gmpxx.h>

struct SuperVector2 {
    long double x;
    long double y;
};

struct ArbVector2 {
    mpf_class x;
    mpf_class y;
};

#endif