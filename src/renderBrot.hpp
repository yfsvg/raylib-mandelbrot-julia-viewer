#ifndef RENDERBROT_HPP
#define RENDERBROT_HPP

#include <vector>
#include <thread>
#include <mutex>
#include <complex>

#include "raylib.h"
#include "raymath.h"

#include "ui_types.hpp"

int isInMandlebrotButGiveIterationsToEscape(std::complex<long double> c, int detailAmt);
void giveMandelbrotOutputsInRange(int startX, int endX, int setWidth, int setHeight, long double centerReal, long double centerImag, std::vector<int>& returnVector, long double zoomFactor, bool usingULDM, bool usingLDM, int detailAmt);
std::vector<Color> calculateEscapeValues(RenderTexture2D mandieSet, long double offsetX, long double offsetY, int threadz, long double zoomFactor, bool usingULDM, bool usingLDM, int detailAmt);

#endif