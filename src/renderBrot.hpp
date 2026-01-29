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

int isInJuliaSet(std::complex<long double> z, std::complex<long double> c, long double power, int detailAmt);
void giveJuliaOutputsInRange(int startX, int endX, int setWidth, int setHeight, long double centerReal, long double centerImag, std::vector<int>& returnVector, long double zoomFactor, std::complex<long double> juliaSeed, long double juliaPower, bool usingULDM, bool usingLDM, int detailAmt);
std::vector<Color> calculateEscapeValuesJulia(RenderTexture2D mandieSet, long double offsetX, long double offsetY, int threadz, long double zoomFactor, std::complex<long double> juliaSeed, long double juliaPower, bool usingULDM, bool usingLDM, int detailAmt);

int isInMandlebrotGMP(mpf_class cReal, mpf_class cImag, int detailAmt);
void giveMandelbrotOutputsInRangeGMP(int startX, int endX, int setWidth, int setHeight, mpf_class centerReal, mpf_class centerImag, std::vector<int>& returnVector, mpf_class arbZoomFactor, bool usingULDM, bool usingLDM, int detailAmt);
std::vector<Color> calculateEscapeValuesForArbs(RenderTexture2D mandieSet, mpf_class offsetX, mpf_class offsetY, int threadz, mpf_class arbZoomFactor, bool usingULDM, bool usingLDM, int detailAmt);

int isInJuliaSetGMP(mpf_class zReal, mpf_class zImag, mpf_class cReal, mpf_class cImag, long double power, int detailAmt);
void giveJuliaOutputsInRangeGMP(int startX, int endX, int setWidth, int setHeight, mpf_class centerReal, mpf_class centerImag, std::vector<int>& returnVector, mpf_class zoomFactor, mpf_class juliaSeedReal, mpf_class juliaSeedImag, long double juliaPower, bool usingULDM, bool usingLDM, int detailAmt);
std::vector<Color> calculateEscapeValuesJuliaArbs(RenderTexture2D mandieSet, mpf_class offsetX, mpf_class offsetY, int threadz, mpf_class zoomFactor, mpf_class juliaSeedReal, mpf_class juliaSeedImag, long double juliaPower, bool usingULDM, bool usingLDM, int detailAmt);

#endif