#include <math.h>
#include <cstdlib>
#include <complex>
#include <vector>
#include <algorithm>
#include <string>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>

#include <gmpxx.h>

#include "ui_types.hpp"
#include "ui.hpp"
#include "renderBrot.hpp"

#include "raylib.h"
#include "raymath.h"


// What I use instead of Vector2! They only go up to floats which isn't super helpful...

SuperVector2 offset = {0, 0};
SuperVector2 renderOffset = {0, 0};
SuperVector2 oldOffset = offset;
ArbVector2 arbOffset = {0, 0};
ArbVector2 arbRenderOffset = {0, 0};
ArbVector2 oldArbOffset = arbOffset;

// To make sure that you don't move as pixels are being found
bool canMove = true;
bool movingRightNow = false;
bool wasMovingLastFrame = false;

long double zoomFactor = 250;
long double oldZoomFactor = zoomFactor;
mpf_class arbZoomFactor = 250;
mpf_class arbOldZoomFactor = arbZoomFactor;
int detailAmt = 250;

bool toggledMoreInfo = true;
float moreInfoOffset = 0;

bool editingDetail = false;
std::string detailInputText = "250";

bool editingZoomSpeed = false;
long double zoomSpeed = 50;

int threadz = 4;
bool editingThreadCount = false;
std::string threadCountInputText = "4";

bool usingArbitraryPrecisionLibrary = false;

bool usingWASD = true;
bool draggingMouse = false;
Vector2 dragStartPos = { 0, 0 };
Vector2 dragEndPos = { 0, 0 };

bool usingLDM = false;
bool usingULDM = false;

bool drawnLoading = false;
bool needToAdjustZoom = false;

int sensitivity = 5;
std::string sensitivityInputText = "5";
bool editingSensitivity = false;

bool showJuliaSet = false;
long double juliaReal = 0.0L;
long double juliaImag = 0.0L;
long double juliaPower = 2.0L;

bool editingJuliaReal = false;
bool editingJuliaImag = false;
bool editingJuliaPower = false;

std::string juliaRealInputText = "0.0";
std::string juliaImagInputText = "0.0";
std::string juliaPowerInputText = "2.0";


// rectangle zoom relocated to main

void drawIntercepts(long double offsetX, long double offsetY) {
    DrawRectangle(GetScreenWidth() / 2 - 1 - (float)offsetX, 0, 2, GetScreenHeight(), RED);
    DrawRectangle(0, GetScreenHeight() / 2 - 1 - (float) offsetY, GetScreenWidth(), 2, RED);
}

std::vector<int> allEscapeValues;
std::vector<Color> relativeEscapeValueColors;

// Rendering, regular

void renderMandelbrotToTexture(RenderTexture2D mandieSet, long double offsetX, long double offsetY) {
    // Only begin texture mode when we are actually ready to draw
    int setWidth = mandieSet.texture.width;
    int setHeight = mandieSet.texture.height;

    if (showJuliaSet) {
        std::complex<long double> juliaSeed(juliaReal, juliaImag);
        relativeEscapeValueColors = calculateEscapeValuesJulia(mandieSet, offsetX, offsetY, threadz, zoomFactor, juliaSeed, juliaPower, usingULDM, usingLDM, detailAmt);
    } else {
        relativeEscapeValueColors = calculateEscapeValues(mandieSet, offsetX, offsetY, threadz, zoomFactor, usingULDM, usingLDM, detailAmt);
    }

    // Draw the resulting colors to the texture
    BeginTextureMode(mandieSet);
    ClearBackground(DARKGRAY);

    int addAmt = 1;

    if (usingULDM) {
        addAmt = 4;
    } else if (usingLDM) {
        addAmt = 2;
    }

    int idx = 0;
    for (int x = 0; x < setWidth; x += addAmt) {
        for (int y = 0; y < setHeight; y += addAmt) {
            if (idx < (int)relativeEscapeValueColors.size()) {
                DrawRectangle(x, y, addAmt, addAmt, relativeEscapeValueColors[idx++]);
            }
        }

    }

    EndTextureMode();
}


// Rendering, arbitrary position library

void renderMandelbrotToTextureArbitraryPrecsion(RenderTexture2D mandieSet, mpf_class offsetX, mpf_class offsetY) {
    int setWidth = mandieSet.texture.width;
    int setHeight = mandieSet.texture.height;

    mpf_class centerReal = offsetX / arbZoomFactor;
    mpf_class centerImag = offsetY / arbZoomFactor;

    allEscapeValues.clear();
    relativeEscapeValueColors.clear();

    relativeEscapeValueColors = calculateEscapeValuesForArbs(mandieSet, offsetX, offsetY, threadz, arbZoomFactor, usingULDM, usingLDM, detailAmt);

    BeginTextureMode(mandieSet);
    ClearBackground(RAYWHITE);

    int addAmt = 1;

    if (usingULDM) {
        addAmt = 4;
    } else if (usingLDM) {
        addAmt = 2;
    }

    int idx = 0;
    for (int x = 0; x < setWidth; x += addAmt) {
        for (int y = 0; y < setHeight; y += addAmt) {
            if (idx < relativeEscapeValueColors.size()) {
                DrawRectangle(x, y, addAmt, addAmt, relativeEscapeValueColors[idx++]);
            }
        }
    }
    EndTextureMode();
}






// Movement, regular

SuperVector2 offsetControls(long double offsetX, long double offsetY) {
    SuperVector2 returnVal = {offsetX, offsetY};
    movingRightNow = false;
    long double speed = 1.0L * 0.2 * sensitivity;

    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
        returnVal.y = offsetY + speed;
        movingRightNow = true;
    }
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
        returnVal.y = offsetY - speed;
        movingRightNow = true;
    }
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        returnVal.x = offsetX - speed;
        movingRightNow = true;
    }
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        returnVal.x = offsetX + speed;
        movingRightNow = true;
    }
    
    return returnVal;
}

// Movement, arbitrary precision

ArbVector2 arbOffsetControls(mpf_class offsetX, mpf_class offsetY) {
    ArbVector2 returnVal = {offsetX, offsetY};
    movingRightNow = false;
    mpf_class speed = 1.0 * 0.2 * sensitivity;

    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
        returnVal.y = offsetY + speed;
        movingRightNow = true;
    }
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
        returnVal.y = offsetY - speed;
        movingRightNow = true;
    }
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        returnVal.x = offsetX - speed;
        movingRightNow = true;
    }
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        returnVal.x = offsetX + speed;
        movingRightNow = true;
    }
    return returnVal;
}


// It took me hours to figure out why there was an issue but apparently to_string has a precision limit too so this method works better
mpf_class longDoubleToMpf(long double toTurn) {
    std::stringstream ss;
    ss << std::setprecision(35) << toTurn;
    std::string e = ss.str();
    return mpf_class(e);
}

// Main loop

int main(void) {
    mpf_set_default_prec(256);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1000, 600, "Mandelbrot");


    // miss mandie
    RenderTexture2D mandelbrotTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    bool needsRedraw = true;
    bool lastFrameWasArb = false;

    SetTargetFPS(120);
    
    long double previousZoom = zoomFactor;
    mpf_class previousArbZoom = arbZoomFactor;
    

    while (!WindowShouldClose()) {
        if (IsWindowResized()) {
            UnloadRenderTexture(mandelbrotTexture);
            mandelbrotTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
            needsRedraw = true;
        }
        
        // Synchronizing measurements whe turning 
        if (usingArbitraryPrecisionLibrary && !lastFrameWasArb) {
            arbOffset.x = longDoubleToMpf(offset.x);
            arbOffset.y = longDoubleToMpf(offset.y);
            arbRenderOffset.x = longDoubleToMpf(renderOffset.x);
            arbRenderOffset.y = longDoubleToMpf(renderOffset.y);
            arbZoomFactor = longDoubleToMpf(zoomFactor);
            arbOldZoomFactor = longDoubleToMpf(oldZoomFactor);
            previousArbZoom = longDoubleToMpf(zoomFactor);
            needsRedraw = true;
        }
        // Synchronizing to regular long double, although precision lost
        else if (!usingArbitraryPrecisionLibrary && lastFrameWasArb) {
            offset.x = arbOffset.x.get_d();
            offset.y = arbOffset.y.get_d();
            renderOffset.x = arbRenderOffset.x.get_d();
            renderOffset.y = arbRenderOffset.y.get_d();
            zoomFactor = arbZoomFactor.get_d();
            oldZoomFactor = arbOldZoomFactor.get_d();
            previousZoom = zoomFactor;
            needsRedraw = true;
        }
        lastFrameWasArb = usingArbitraryPrecisionLibrary;




        // Movement



        if (canMove && usingWASD) {
            if (usingArbitraryPrecisionLibrary) {
                 arbOffset = arbOffsetControls(arbOffset.x, arbOffset.y);
                 
                 if (IsKeyDown(KEY_M)) {
                    arbZoomFactor += arbZoomFactor / longDoubleToMpf(zoomSpeed);
                    movingRightNow = true;
                }

                if (IsKeyDown(KEY_N)) {
                    arbZoomFactor -= arbZoomFactor / longDoubleToMpf(zoomSpeed);
                    movingRightNow = true;
                }

                if (IsKeyPressed(KEY_M) || IsKeyPressed(KEY_N)) {
                    arbOldZoomFactor = arbZoomFactor;
                }

                 if (wasMovingLastFrame && !movingRightNow) {
                    needToAdjustZoom = true;
                    needsRedraw = true;
                }

            } else {
                offset = offsetControls(offset.x, offset.y);
                
                if (IsKeyDown(KEY_M)) {
                    zoomFactor += zoomFactor / zoomSpeed;
                    movingRightNow = true;
                }
                if (IsKeyDown(KEY_N)) {
                    zoomFactor -= zoomFactor / zoomSpeed;
                    movingRightNow = true;
                }

                if (IsKeyPressed(KEY_M) || IsKeyPressed(KEY_N)) {
                    oldZoomFactor = zoomFactor;
                }
                
                if (wasMovingLastFrame && !movingRightNow) {
                    needToAdjustZoom = true;
                    needsRedraw = true;
                }

            }
        }
        
        
        if (canMove && !usingWASD) {
            movingRightNow = false;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                oldOffset = offset;
                dragStartPos = GetMousePosition();
                draggingMouse = true;
            }

            if (draggingMouse && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                dragEndPos = GetMousePosition();
                offset.x = oldOffset.x + (dragStartPos.x - dragEndPos.x);
                offset.y = oldOffset.y + (dragStartPos.y - dragEndPos.y);
                movingRightNow = true;
            }

            if (draggingMouse && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                dragEndPos = GetMousePosition();
                offset.x = oldOffset.x + (dragStartPos.x - dragEndPos.x);
                offset.y = oldOffset.y + (dragStartPos.y - dragEndPos.y);
                draggingMouse = false;
            }

            if (IsKeyDown(KEY_M)) {
                zoomFactor += zoomFactor / zoomSpeed;
                movingRightNow = true;
            }
            if (IsKeyDown(KEY_N)) {
                zoomFactor -= zoomFactor / zoomSpeed;
                movingRightNow = true;
            }

            if (wasMovingLastFrame && !movingRightNow) {
                needToAdjustZoom = true;
                
                oldZoomFactor = zoomFactor;
                if (std::abs(dragStartPos.x - dragEndPos.x) < 3 && std::abs(dragStartPos.y - dragEndPos.y) < 3) {
                    offset = oldOffset;
                    needToAdjustZoom = false; 
                } else {
                    needsRedraw = true;
                }
                
            }
            

        }




        // Drawing


        if (needsRedraw && !movingRightNow) {
            if (!drawnLoading) {
                drawnLoading = true; 
            } else {
                canMove = false;

                if (needToAdjustZoom) {
                    if (usingArbitraryPrecisionLibrary) {
                        mpf_class zoomRatio = arbZoomFactor / previousArbZoom;
                        arbOffset.x = arbOffset.x * zoomRatio;
                        arbOffset.y = arbOffset.y * zoomRatio;
                        previousArbZoom = arbZoomFactor;
                        arbOldZoomFactor = arbZoomFactor;
                    } else {
                        if (fabsl(zoomFactor - previousZoom) > 0.01L) {
                            offset.x = offset.x * (zoomFactor / previousZoom);
                            offset.y = offset.y * (zoomFactor / previousZoom);
                            previousZoom = zoomFactor;
                        }
                        oldZoomFactor = zoomFactor;
                    }
                    needToAdjustZoom = false;
                }

                if (usingArbitraryPrecisionLibrary) {
                    renderMandelbrotToTextureArbitraryPrecsion(mandelbrotTexture, arbOffset.x, arbOffset.y);
                    arbRenderOffset = arbOffset;
                } else {
                    renderMandelbrotToTexture(mandelbrotTexture, offset.x, offset.y);
                    renderOffset = offset;
                }
                needsRedraw = false;
                canMove = true;
                drawnLoading = false; 
            }
        }
        
        
        wasMovingLastFrame = movingRightNow;

        BeginDrawing();
        ClearBackground(RAYWHITE);


        float texX, texY;
        if (usingArbitraryPrecisionLibrary) {
            mpf_class diffX = arbRenderOffset.x - arbOffset.x;
            mpf_class diffY = arbRenderOffset.y - arbOffset.y;
            texX = (float)diffX.get_d();
            texY = (float)diffY.get_d();
        } else {
            texX = (float)(renderOffset.x - offset.x);
            texY = (float)(renderOffset.y - offset.y);
        }

        Vector2 texturePos = { texX, texY };
        
        DrawTextureRec(mandelbrotTexture.texture, (Rectangle){0, 0, (float)mandelbrotTexture.texture.width, (float)-mandelbrotTexture.texture.height}, texturePos, WHITE);
        
        if (movingRightNow || drawnLoading) {
            long double scale;
            if (usingArbitraryPrecisionLibrary) {
                 mpf_class s = arbOldZoomFactor / arbZoomFactor;
                 scale = s.get_d();
            } else {
                 scale = oldZoomFactor / zoomFactor;
            }

              
                scale = 1 / scale;
                Rectangle origMandie = {0, 0, (float)mandelbrotTexture.texture.width, (float)-mandelbrotTexture.texture.height};
                Rectangle zoomChangeMandie = {
                    (float)(GetScreenWidth() / 2 - (mandelbrotTexture.texture.width * scale) / 2 + texturePos.x * scale),
                    (float)(GetScreenHeight() / 2 - (mandelbrotTexture.texture.height * scale) / 2 + texturePos.y * scale),
                    (float)(mandelbrotTexture.texture.width * scale),
                    (float)(mandelbrotTexture.texture.height * scale)
                };
                DrawTexturePro(mandelbrotTexture.texture, origMandie, zoomChangeMandie, (Vector2){0, 0}, 0.0f, WHITE);
        }

        if (IsKeyPressed(KEY_I)) {
            toggledMoreInfo = !toggledMoreInfo;
        }
        if (toggledMoreInfo) {
            moreInfoOffset += (0 - moreInfoOffset) / 10;
        } else {
            moreInfoOffset += (-320 - moreInfoOffset) / 10;
        }

        // MORE INFO display
        Rectangle moreInfoBox;
        if (showJuliaSet) {
            moreInfoBox = { 5 + moreInfoOffset, 5, 300, 500 };
        } else {
            moreInfoBox = { 5 + moreInfoOffset, 5, 300, 370 };
        }
        DrawRectangleRounded(moreInfoBox, 0.00f, 4, Fade(BLACK, 0.8f));
        DrawRectangleLinesEx(moreInfoBox, 1, Fade(WHITE, 0.5f));


        Rectangle collapseButton = {304 + moreInfoOffset, 15, 30, 50};
        DrawRectangleRec(collapseButton, Fade(BLACK, 0.8f));
        DrawRectangleLinesEx(collapseButton, 1, Fade(WHITE, 0.5f));
        if (CheckCollisionPointRec(GetMousePosition(), collapseButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            editingDetail = false;
            editingThreadCount = false;
            toggledMoreInfo = !toggledMoreInfo;
        }



        
        // UI

        DrawFPS(15 + moreInfoOffset, 15);
        
        drawPosition(usingArbitraryPrecisionLibrary, zoomFactor, offset, moreInfoOffset, arbOffset, arbZoomFactor);

        int lengthInput;
        std::vector<bool*> allEditingStates = {&editingDetail, &editingZoomSpeed, &editingThreadCount, &editingSensitivity, &editingJuliaReal, &editingJuliaImag, &editingJuliaPower};
        
        drawTextInput(moreInfoOffset, 15, 100, 100, 30,
                editingDetail, allEditingStates,
                detailInputText, "Iterations", needsRedraw,
                detailAmt);

        drawCheckbox(usingArbitraryPrecisionLibrary, moreInfoOffset, 15, 135, 30, 30, "Arbitrary Precision", 5);

        drawTextInput(moreInfoOffset, 15, 170, 100, 30,
                editingSensitivity, allEditingStates,
                sensitivityInputText, "Sensitivity", needsRedraw,
                sensitivity);

        drawTextInput(moreInfoOffset, 15, 205, 100, 30,
                editingThreadCount, allEditingStates,
                threadCountInputText, "Threads", needsRedraw,
                threadz);
        
        drawCheckbox(usingLDM, moreInfoOffset, 15, 240, 30, 30, "Low Detail Mode", 5);
        drawCheckbox(usingULDM, moreInfoOffset, 15, 275, 30, 30, "Ultra LDM", 5);

        drawCheckbox(usingWASD, moreInfoOffset, 15, 310, 30, 30, "Using WASD", 5);

        drawCheckbox(showJuliaSet, moreInfoOffset, 15, 345, 30, 30, "Julia Set", 5);

        if (showJuliaSet) {
            drawTextInputDouble(moreInfoOffset, 15, 380, 100, 30,
                    editingJuliaReal, allEditingStates,
                    juliaRealInputText, "Re", needsRedraw,
                    juliaReal);

            drawTextInputDouble(moreInfoOffset, 15, 415, 100, 30,
                    editingJuliaImag, allEditingStates,
                    juliaImagInputText, "Im", needsRedraw,
                    juliaImag);

            drawTextInputDouble(moreInfoOffset, 15, 450, 100, 30,
                    editingJuliaPower, allEditingStates,
                    juliaPowerInputText, "Power", needsRedraw,
                    juliaPower);
        }



        if (drawnLoading == true) {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.3f));
            DrawText("Getting Fractal Pixels", GetScreenWidth() / 2 - 100, GetScreenHeight() / 2 - 30, 20, RAYWHITE);
            DrawRectangle(GetScreenWidth() / 2 - 100, GetScreenHeight() / 2, 200, 20, BLACK);
            DrawRectangle(GetScreenWidth() / 2 - 95, GetScreenHeight() / 2 + 5, 190, 10, RAYWHITE);
        }

        EndDrawing();
    }
    
    UnloadRenderTexture(mandelbrotTexture);
    CloseWindow();
    return 0;
}