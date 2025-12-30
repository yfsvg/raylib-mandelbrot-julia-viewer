#ifndef UI_HPP
#define UI_HPP

#include <sstream>
#include <iomanip>

#include "raylib.h"
#include "raymath.h"

#include "ui_types.hpp"

void drawFPSRegular(long double zoomFactor, SuperVector2 offset, float moreInfoOffset);

#endif