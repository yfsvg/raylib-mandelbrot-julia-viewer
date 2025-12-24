// Improvements made:
// -----------------------
// Logarithmic zoom
// Moving entire texture instead of recalculating
// Float -> Double
// Smart redrawing
// Double -> long double
// Local coordinates
// Made shading work!
// Better UI

// Optimizations / features to make:
// Loading screen so that you dont move during calculations
// Even better UI with more checkboxes
// FIX PERIODICTY CODE!!!!
// Add a number line, mouse pointer coord display
// Paramaterize z, make Julia set




#include <math.h>
#include <cstdlib>
#include <complex>
#include <vector>
#include <algorithm>
#include <string>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "raylib.h"
#include "raymath.h"

// What I use instead of Vector2! They only go up to floats which isn't super helpful...
struct SuperVector2 {
    long double x;
    long double y;
};

// To make sure that you don't move as pixels are being found
bool canMove = true;
bool movingRightNow = false;
bool wasMovingLastFrame = false;

long double zoomFactor = 250;
long double oldZoomFactor = zoomFactor;
int detailAmt = 250;

bool toggledMoreInfo = true;

bool editingDetail = false;
char detailInputText[6] = "250";

bool editingZoomSpeed = false;
char zoomSpeedInputText[6] = "50";
long double zoomSpeed = 50;

bool usingBoxZoom = false;
/*
Color yefoiGrey = (Color){ 27, 27, 27 };
*/

// rectangle zoom relocated to main

void drawIntercepts(long double offsetX, long double offsetY) {
    DrawRectangle(GetScreenWidth() / 2 - 1 - (float)offsetX, 0, 2, GetScreenHeight(), RED);
    DrawRectangle(0, GetScreenHeight() / 2 - 1 - (float) offsetY, GetScreenWidth(), 2, RED);
}

// Apparently the periodicty code doesnt work AT ALL! Be sure to look into ts later
bool isInMandelbrot(std::complex<long double> c) {
    // c is the complex parameter/number that is being tested to see if its in the set
    // z is the current iteration of the mandelbrot result, and z_old is the previous one, used to detect periodicity and such

    std::complex<long double> z(0.0L, 0.0L);
    // Figuring out if the thing is in the main cardiod or not
    if ((c.real() + 0.25L) * (c.real() + 0.25L) + c.imag() * c.imag() < 0.25L) {
        return true;
    }
    // Figuring out if its in the second cardiod
    if ((c.real() + 1.0L) * (c.real() + 1.0L) + c.imag() * c.imag() < 0.06L) {
        return true;
    }
    // Figuring out if the x val is over 0.5
    if (c.real() > 0.5L) {
        return false;
    }
    // Figuring out if the y val is over 1.2 (Imaginary)
    if (std::abs(c.imag()) > 1.2L) {
        return false;
    }
    // These early breaks might be helpful for bigger pictures maybe by like a few percent lol

    std::complex<long double> z_old(0.0L, 0.0L);
    int period = 0;
    for (int i = 0; i < detailAmt; i++) {
        z = z * z + c;
        if (std::abs(z) > 2.0L) {
            return false; 
        }
        if (i == period) {
            if (std::abs(z - z_old) < 1e-10L) {
                return true;
            }
            z_old = z;
            period = i;
        }
    }
    return true; 
}

// Exact same code as above, HOWEVER, it returns the amount of iterations needed to escape.
// If it doesn't escape with the given detailAmt, then it's marked as -1
int isInMandlebrotButGiveIterationsToEscape(std::complex<long double> c) {
    std::complex<long double> z(0.0L, 0.0L);
    std::complex<long double> z_old(0.0L, 0.0L);
    int period = 0;
    for (int i = 0; i < detailAmt; i++)
    {
        z = z * z + c;
        if (std::abs(z) > 2.0L)
        {
            return i + 1;
        }
        if (i == period)
        {
            if (std::abs(z - z_old) < 1e-10L)
            {
                return -1;
            }
            z_old = z;
            period = i;
        }
    }
    return -1;

    
}

std::vector<int> allEscapeValues;
std::vector<Color> relativeEscapeValueColors;

// Changed the implementation of the way that the texture is shown, instead of calculating everything
// Only redraw the texture after each change in perspective
void renderMandelbrotToTexture(RenderTexture2D mandieSet, long double offsetX, long double offsetY) {
    BeginTextureMode(mandieSet);
    ClearBackground(RAYWHITE);

    int setWidth = mandieSet.texture.width;
    int setHeight = mandieSet.texture.height;

    // Calculating the center in complex world using local coordinates
    long double centerReal = offsetX / zoomFactor;
    long double centerImag = offsetY / zoomFactor;

    allEscapeValues.clear();
    relativeEscapeValueColors.clear();
    for (int x = setWidth * -0.5; x <= setWidth * 0.5; x++) {
        for (int y = setHeight * -0.5; y <= setHeight * 0.5; y++) {
            long double real = centerReal + x / zoomFactor;
            long double imag = centerImag + y / zoomFactor;

            allEscapeValues.push_back(isInMandlebrotButGiveIterationsToEscape(std::complex<long double>(real, imag)));
        }
    }

    // Find min and max escape values because the coloring is relative
    int minimumEscapeIterations = INT_MAX;
    int maximumEscapeIterations = INT_MIN;
    for (size_t i = 0; i < allEscapeValues.size(); i++) {
        int val = allEscapeValues[i];
        if (val != -1) {
            minimumEscapeIterations = std::min(minimumEscapeIterations, val);
            maximumEscapeIterations = std::max(maximumEscapeIterations, val);
        }
    }

    // Coloring the colors based on escape values
    for (size_t i = 0; i < allEscapeValues.size(); i++) {
        int val = allEscapeValues[i];

        if (val == -1) {
            relativeEscapeValueColors.push_back(BLACK);
        } else {
            float t = (float)(val - minimumEscapeIterations) / (maximumEscapeIterations - minimumEscapeIterations);

            relativeEscapeValueColors.push_back(
                ColorFromHSV(t * 360.0f, 1.0f, 1.0f)
            );
        }
    }


    int idx = 0;
    for (int x = setWidth * -0.5; x <= setWidth * 0.5; x++) {
        for (int y = setHeight * -0.5; y <= setHeight * 0.5; y++) {
            DrawRectangle(setWidth / 2 + x, setHeight / 2 + y, 1, 1, relativeEscapeValueColors[idx++]);
        }
    }

        // If the heat map is on, then use the isInMandlebrotButGiveIterationsToEscape function, not implemented yet but this is how i want to do it:
        // Go through each pixel, just like the nested for loops below

    EndTextureMode();
}

SuperVector2 offsetControls(long double offsetX, long double offsetY, RenderTexture2D mandelbrotTexture, SuperVector2 *renderOffset) {
    SuperVector2 returnVal = {offsetX, offsetY};
    movingRightNow = false;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
        returnVal.y = offsetY + 1.0L;
        movingRightNow = true;
    }
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
        returnVal.y = offsetY - 1.0L;
        movingRightNow = true;
    }
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        returnVal.x = offsetX - 1.0L;
        movingRightNow = true;
    }
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        returnVal.x = offsetX + 1.0L;
        movingRightNow = true;
    }
    
    return returnVal;
}

// For easier displaying on the info side
std::string truncateZeroes(long double input, int truncateAmount) {
    std::stringstream ss;
    // truncates
    ss << std::setprecision(truncateAmount) << input;
    std::string s_value = ss.str();

    if (s_value.find('.') != std::string::npos) {
        s_value.erase(s_value.find_last_not_of('0') + 1, std::string::npos);
        if (s_value.back() == '.') {
            s_value.pop_back();
        }
    }

    return s_value;
}

int main(void) {
    SuperVector2 offset = {0, 0};
    SuperVector2 renderOffset = {0, 0}; 
    InitWindow(900, 400, "Mandelbrot");
    
    // miss mandie
    RenderTexture2D mandelbrotTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    bool needsRedraw = true;

    SetTargetFPS(120);
    
    long double previousZoom = zoomFactor;
    
    while (!WindowShouldClose()) {
        if (canMove) {
            SuperVector2 newOffset = offsetControls(offset.x, offset.y, mandelbrotTexture, &renderOffset);
            offset = newOffset;
            
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
            
            // Check if movement/zoom just ended this frame
            if (wasMovingLastFrame && !movingRightNow) {
                // Apply zoom offset adjustment
                if (fabsl(zoomFactor - previousZoom) > 0.01L) {
                    long double zoomRatio = zoomFactor / previousZoom;
                    offset.x = offset.x * zoomRatio;
                    offset.y = offset.y * zoomRatio;
                    previousZoom = zoomFactor;
                }
                
                oldZoomFactor = zoomFactor;
                needsRedraw = true;
            }
        }
        
        if (IsKeyPressed(KEY_I)) {
            toggledMoreInfo = !toggledMoreInfo;
        }

        if (needsRedraw && !movingRightNow) {
            canMove = false;
            renderMandelbrotToTexture(mandelbrotTexture, offset.x, offset.y);
            renderOffset = offset;
            needsRedraw = false;
            canMove = true;
        }
        
        
        // Store movement state for next frame
        wasMovingLastFrame = movingRightNow;

        BeginDrawing();
        ClearBackground(RAYWHITE);

        Vector2 texturePos = {
            (float)(renderOffset.x - offset.x),
            (float)(renderOffset.y - offset.y)
        };
        DrawTextureRec(mandelbrotTexture.texture, 
                        (Rectangle){0, 0, (float)mandelbrotTexture.texture.width, (float)-mandelbrotTexture.texture.height}, 
                        texturePos, WHITE);
        if (movingRightNow) {
            long double scale = oldZoomFactor / zoomFactor;
            if (usingBoxZoom) {
                Rectangle outline = {.x = (float)((1 - scale) * GetScreenWidth() / 2), .y = (float)((1 - scale) * GetScreenHeight() / 2), .width = (float)(scale * GetScreenWidth()), .height = (float)(scale * GetScreenHeight())};
            } else {               
                // shrunked down scaled texture
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
        }


        // MORE INFO display
        if (toggledMoreInfo) {
            Rectangle moreInfoBox = { 5, 5, 300, 300 };
            DrawRectangleRounded(moreInfoBox, 0.1f, 16, Fade(BLACK, 0.8f));

            DrawFPS(15, 15);
            DrawText(TextFormat("Zoom: %s", truncateZeroes(zoomFactor, 15).c_str()), 15, 35, 20, WHITE);
            DrawText(TextFormat("X Pos: %s", truncateZeroes(offset.x / zoomFactor, 10).c_str()), 15, 55, 20, WHITE);
            DrawText(TextFormat("Y Pos: %s", truncateZeroes(offset.y / zoomFactor, 10).c_str()), 15, 75, 20, WHITE);

            int lengthInput;
            // Textboxes and checkboxes options
            // if statement is necessary because the textboxes have a lot of code that i don'T want to see all the time
            if (true) {
                // Iteration amount input area
                Rectangle detailInputBox = { 15, 100, 100, 30 };
                if (editingDetail) {
                    DrawRectangleRec(detailInputBox, Fade(WHITE, 0.3f));
                } else {
                    DrawRectangleRec(detailInputBox, Fade(WHITE, 0.1f));
                }
                DrawText(detailInputText, 20, 105, 20, WHITE);
                DrawText("Iterations", 125, 105, 20, WHITE);

                if (CheckCollisionPointRec(GetMousePosition(), detailInputBox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    editingDetail = true;
                }
                if (editingDetail) {
                    int key = GetCharPressed();
                    lengthInput = strlen(detailInputText);
                    if (key >= '0' && key <= '9' && lengthInput < 5) {
                        detailInputText[lengthInput] = (char)key;
                        detailInputText[lengthInput + 1] = '\0';
                    }
                    if (IsKeyPressed(KEY_BACKSPACE) && lengthInput > 0) {
                        detailInputText[lengthInput - 1] = '\0';
                    }
                    if (IsKeyPressed(KEY_ENTER)) {
                        detailAmt = atoi(detailInputText);
                        if (detailAmt < 10) detailAmt = 10;
                        needsRedraw = true;
                        editingDetail = false;
                    }
                }

                // Editing zoom speed
                Rectangle zoomSpeedInputBox = { 15, 135, 100, 30 };
                if (editingZoomSpeed) {
                    DrawRectangleRec(zoomSpeedInputBox, Fade(WHITE, 0.3f));
                } else {
                    DrawRectangleRec(zoomSpeedInputBox, Fade(WHITE, 0.1f));
                }
                DrawText(zoomSpeedInputText, 20, 140, 20, WHITE);
                DrawText("Zoom speed", 125, 140, 20, WHITE);

                if (CheckCollisionPointRec(GetMousePosition(), zoomSpeedInputBox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    editingZoomSpeed = true;
                }
                if (editingZoomSpeed) {
                    int key = GetCharPressed();
                    lengthInput = strlen(zoomSpeedInputText);
                    if (key >= '0' && key <= '9' && lengthInput < 5) {
                        zoomSpeedInputText[lengthInput] = (char)key;
                        zoomSpeedInputText[lengthInput + 1] = '\0';
                    }
                    if (IsKeyPressed(KEY_BACKSPACE) && lengthInput > 0) {
                        zoomSpeedInputText[lengthInput - 1] = '\0';
                    }
                    if (IsKeyPressed(KEY_ENTER)) {
                        zoomSpeed = atoi(zoomSpeedInputText);
                        if (zoomSpeed < 5) zoomSpeed = 5;
                        if (zoomSpeed > 200) zoomSpeed = 200;
                        // This seems kind of strage but turning up zoomspeed actually makes it slower for some reason lol but this is more intuitive
                        zoomSpeed = (200 / zoomSpeed) * 5 * 2.5f;
                        editingZoomSpeed = false;
                    }
                }
            }

        }

        EndDrawing();
    }
    
    UnloadRenderTexture(mandelbrotTexture);
    CloseWindow();
    return 0;
}