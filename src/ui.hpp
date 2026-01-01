#ifndef UI_HPP
#define UI_HPP

#include <sstream>
#include <iomanip>

#include "raylib.h"
#include "raymath.h"

#include "ui_types.hpp"

void drawPosition(bool usingArbitraryPrecisionLibrary, long double zoomFactor, SuperVector2 offset, float moreInfoOffset, ArbVector2 arbOffset, mpf_class arbZoomFactor);
void drawCheckbox(bool& variableToChange, float moreInfoOffset, float x, float y, float sizeX, float sizeY, std::string textWithBox, float margins);

void drawTextInput(float moreInfoOffset, float x, float y, float sizeX, float sizeY,
                    bool& editingCurrentVariable, bool& editingOV1, bool& editingOV2,
                    std::string& inputText, std::string queryText, bool& needsRedraw,
                    int& variableToChange
);

#endif