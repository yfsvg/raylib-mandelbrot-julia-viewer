// next implementing mouse dragging and scrolling functionality, as well as a delay to render a new screen so users have time to breathe
// In between the time it takes for the user to breathe add a loading bar, same for as the rendering process.



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


#include "raylib.h"
#include "raymath.h"


// What I use instead of Vector2! They only go up to floats which isn't super helpful...
struct SuperVector2 {
    long double x;
    long double y;
};

struct ArbVector2 {
    mpf_class x;
    mpf_class y;
};

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
char detailInputText[6] = "250";

bool editingZoomSpeed = false;
char zoomSpeedInputText[6] = "50";
long double zoomSpeed = 50;

bool usingBoxZoom = false;

int threadz = 4;
bool editingThreadCount = false;
char threadCountInputText[6] = "4";

bool usingArbitraryPrecisionLibrary = false;

int detailLevelUsed = 4;


// rectangle zoom relocated to main

void drawIntercepts(long double offsetX, long double offsetY) {
    DrawRectangle(GetScreenWidth() / 2 - 1 - (float)offsetX, 0, 2, GetScreenHeight(), RED);
    DrawRectangle(0, GetScreenHeight() / 2 - 1 - (float) offsetY, GetScreenWidth(), 2, RED);
}


// Exact same code as above, HOWEVER, it returns the amount of iterations needed to escape.
// If it doesn't escape with the given detailAmt, then it's marked as -1
int isInMandlebrotButGiveIterationsToEscape(std::complex<long double> c) {
    std::complex<long double> z(0.0L, 0.0L);
    std::complex<long double> z_old(0.0L, 0.0L);
    int period = 0;
    for (int i = 0; i < detailAmt; i++) {
        z = z * z + c;
        if (std::abs(z) > 2.0L) {
            return i + 1;
        }
        if (i == period) {
            if (std::abs(z - z_old) < 1e-10L) {
                return -1;
            }
            z_old = z;
            period = i;
        }
    }
    return -1;
}

int isInMandlebrotGMP(mpf_class cReal, mpf_class cImag) {
    mpf_class zReal = 0, zImag = 0;
    mpf_class z_r_sq = 0, z_i_sq = 0, temp;
    
    // Periodicity checking ended up being slower than just ignoring it
    for (int i = 0; i < detailAmt; i++) {
        temp = z_r_sq - z_i_sq + cReal;
        zImag = 2 * zReal * zImag + cImag;
        zReal = temp;
        
        z_r_sq = zReal * zReal;
        z_i_sq = zImag * zImag;
        
        if (z_r_sq + z_i_sq > 4.0) {
            return i + 1;
        }
    }
    return -1;
}

std::vector<int> allEscapeValues;
std::vector<Color> relativeEscapeValueColors;

void giveMandelbrotOutputsInRange(int startX, int endX, int setWidth, int setHeight, long double centerReal, long double centerImag, std::vector<int>& returnVector) {
    // performance smiling
    returnVector.reserve((endX - startX) * setHeight);
    for (int x = startX; x < endX; x++) {
        for (int y = 0; y < setHeight; y++) {
            // converting 0-based screen coordinates because the function demands actual mathematical coords
            long double real = centerReal + ((x - setWidth/2.0) / zoomFactor);
            long double imag = centerImag + ((y - setHeight/2.0) / zoomFactor);
            returnVector.push_back(isInMandlebrotButGiveIterationsToEscape(std::complex<long double>(real, imag)));
        }
    }
}

void giveMandelbrotOutputsInRangeGMP(int startX, int endX, int setWidth, int setHeight, mpf_class centerReal, mpf_class centerImag, std::vector<int>& returnVector) {
    returnVector.reserve((endX - startX) * setHeight);
    
    mpf_class real, imag;
    mpf_class step = 1.0 / arbZoomFactor; // Pre-calc step to save divisions
    
    for (int x = startX; x < endX; x++) {
        real = centerReal + (x - setWidth/2.0) * step;
        for (int y = 0; y < setHeight; y++) {
            imag = centerImag + (y - setHeight/2.0) * step;
            returnVector.push_back(isInMandlebrotGMP(real, imag));
        }
    }
}

void renderMandelbrotToTexture(RenderTexture2D mandieSet, long double offsetX, long double offsetY) {
    // Only begin texture mode when we are actually ready to draw
    int setWidth = mandieSet.texture.width;
    int setHeight = mandieSet.texture.height;

    // Calculating the center in complex world using local coordinates
    long double centerReal = offsetX / zoomFactor;
    long double centerImag = offsetY / zoomFactor;

    allEscapeValues.clear();
    relativeEscapeValueColors.clear();

    // Multi-threading setup
    std::vector<std::vector<int>> tLists(threadz);
    int stripWidth = setWidth / threadz;
    std::vector<std::thread> threadsList;

    for (int i = 0; i < threadz; i++) {
        threadsList.push_back(std::thread([&, i]() {
            int endPosition;
            if (i == threadz - 1) {
                endPosition = setWidth;
            } else {
                endPosition = stripWidth * (i + 1);
            }

            giveMandelbrotOutputsInRange(stripWidth * i, endPosition, setWidth, setHeight, centerReal, centerImag, tLists[i]);
        }));
    }

    for (int i = 0; i < threadsList.size(); i++) {
        threadsList[i].join();
    }

    for (int i = 0; i < tLists.size(); i++) {
        allEscapeValues.insert(allEscapeValues.end(), tLists[i].begin(), tLists[i].end());
    }

    int minimumEscapeIterations = INT_MAX;
    int maximumEscapeIterations = INT_MIN;

    for (size_t i = 0; i < allEscapeValues.size(); i++) {
        if (allEscapeValues[i] != -1) {
            if (allEscapeValues[i] < minimumEscapeIterations) {
                minimumEscapeIterations = allEscapeValues[i];
            }
            if (allEscapeValues[i] > maximumEscapeIterations) {
                maximumEscapeIterations = allEscapeValues[i];
            }
        }
    }

    // Calculate colors based on escape values (HSV mapping)
    for (int val : allEscapeValues) {
        if (val == -1) {
            relativeEscapeValueColors.push_back(BLACK);
        } else {
            float range = (float)(maximumEscapeIterations - minimumEscapeIterations);
            if (range == 0) range = 1.0f;


            float t = (float)(val - minimumEscapeIterations) / range;
            relativeEscapeValueColors.push_back(ColorFromHSV(t * 360.0f, 1.0f, 1.0f));
        }
    }

    // Draw the resulting colors to the texture
    BeginTextureMode(mandieSet);
    ClearBackground(RAYWHITE);

    int idx = 0;
    for (int x = 0; x < setWidth; x++) {
        for (int y = 0; y < setHeight; y++) {
            if (idx < (int)relativeEscapeValueColors.size()) {
                DrawRectangle(x, y, 1, 1, relativeEscapeValueColors[idx++]);
            }
        }
    }

    EndTextureMode();
}


void renderMandelbrotToTextureArbitraryPrecsion(RenderTexture2D mandieSet, mpf_class offsetX, mpf_class offsetY) {
    int setWidth = mandieSet.texture.width;
    int setHeight = mandieSet.texture.height;

    mpf_class centerReal = offsetX / arbZoomFactor;
    mpf_class centerImag = offsetY / arbZoomFactor;

    allEscapeValues.clear();
    relativeEscapeValueColors.clear();

    std::vector<std::vector<int>> tLists(threadz);
    int stripWidth = setWidth / threadz;

    std::vector<std::thread> threadsList;
    for (int i = 0; i < threadz; i++) {
        threadsList.push_back(std::thread([&, i]() { 
            int endPosition;
            if (i == threadz - 1) {
                endPosition = setWidth;
            } else {
                endPosition = stripWidth * (i + 1);
            }
            giveMandelbrotOutputsInRangeGMP(stripWidth * i, endPosition, setWidth, setHeight, centerReal, centerImag, tLists[i]); 
        }));
    }

    for (int i = 0; i < threadsList.size(); i++) {
        threadsList[i].join();
    }

    for (int i = 0; i < tLists.size(); i++) {
        allEscapeValues.insert(allEscapeValues.end(), tLists[i].begin(), tLists[i].end());
    }

    // reusing coloring logic (identical to float version)
    int minimumEscapeIterations = INT_MAX;
    int maximumEscapeIterations = INT_MIN;
    for (size_t i = 0; i < allEscapeValues.size(); i++) {
        if (allEscapeValues[i] != -1) {
            if (allEscapeValues[i] < minimumEscapeIterations) {
                minimumEscapeIterations = allEscapeValues[i];
            }
            if (allEscapeValues[i] > maximumEscapeIterations) {
                maximumEscapeIterations = allEscapeValues[i];
            }
        }
    }

    for (int val : allEscapeValues) {
        if (val == -1) {
            relativeEscapeValueColors.push_back(BLACK);
        } else {
            float range = (float) (maximumEscapeIterations - minimumEscapeIterations);
            if (range == 0) {
                range = 1.0f; 
            }
            float t = (float) (val - minimumEscapeIterations) / range;
            relativeEscapeValueColors.push_back(ColorFromHSV(t * 360.0f, 1.0f, 1.0f));
        }
    }

    BeginTextureMode(mandieSet);
    ClearBackground(RAYWHITE);
    int idx = 0;
    for (int x = 0; x < setWidth; x++) {
        for (int y = 0; y < setHeight; y++) {
            if (idx < relativeEscapeValueColors.size()) {
                DrawRectangle(x, y, 1, 1, relativeEscapeValueColors[idx++]);
            }
        }
    }
    EndTextureMode();
}


SuperVector2 offsetControls(long double offsetX, long double offsetY, RenderTexture2D mandelbrotTexture, SuperVector2 *renderOffset) {
    SuperVector2 returnVal = {offsetX, offsetY};
    movingRightNow = false;
    long double speed = 1.0L;

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

ArbVector2 arbOffsetControls(mpf_class offsetX, mpf_class offsetY, RenderTexture2D mandelbrotTexture, ArbVector2 *renderOffset) {
    ArbVector2 returnVal = {offsetX, offsetY};
    movingRightNow = false;
    mpf_class speed = 1.0;

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
// Using massive amounts 


// It took me hours to figure out why there was an issue but apparently to_string has a precision limit too so this method works better
mpf_class longDoubleToMpf(long double toTurn) {
    std::stringstream ss;
    ss << std::setprecision(35) << toTurn;
    std::string e = ss.str();
    return mpf_class(e);
}

int main(void) {
    // Increase default precision for GMP
    mpf_set_default_prec(256);

    SuperVector2 offset = {0, 0};
    SuperVector2 renderOffset = {0, 0};
    ArbVector2 arbOffset = {0, 0};
    ArbVector2 arbRenderOffset = {0, 0};

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1200, 700, "Mandelbrot");


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
            // i love getting d
            renderOffset.x = arbRenderOffset.x.get_d();
            renderOffset.y = arbRenderOffset.y.get_d();
            zoomFactor = arbZoomFactor.get_d();
            oldZoomFactor = arbOldZoomFactor.get_d();
            previousZoom = zoomFactor;
            needsRedraw = true;
        }
        lastFrameWasArb = usingArbitraryPrecisionLibrary;


        if (canMove) {
            if (usingArbitraryPrecisionLibrary) {
                 arbOffset = arbOffsetControls(arbOffset.x, arbOffset.y, mandelbrotTexture, &arbRenderOffset);
                 
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
                    mpf_class zoomRatio = arbZoomFactor / previousArbZoom;
                    arbOffset.x = arbOffset.x * zoomRatio;
                    arbOffset.y = arbOffset.y * zoomRatio;
                    previousArbZoom = arbZoomFactor;
                    arbOldZoomFactor = arbZoomFactor;
                    needsRedraw = true;
                }

            } else {


                offset = offsetControls(offset.x, offset.y, mandelbrotTexture, &renderOffset);
                
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
        }
    

        if (needsRedraw && !movingRightNow) {
            canMove = false;
            if (usingArbitraryPrecisionLibrary) {
                renderMandelbrotToTextureArbitraryPrecsion(mandelbrotTexture, arbOffset.x, arbOffset.y);
                arbRenderOffset = arbOffset;
            } else {
                renderMandelbrotToTexture(mandelbrotTexture, offset.x, offset.y);
                renderOffset = offset;
            }
            needsRedraw = false;
            canMove = true;
        }
        
        
        wasMovingLastFrame = movingRightNow;

        BeginDrawing();
        ClearBackground(RAYWHITE);


        float texX, texY;
        if (usingArbitraryPrecisionLibrary) {
            mpf_class diffX = arbRenderOffset.x - arbOffset.x;
            mpf_class diffY = arbRenderOffset.y - arbOffset.y;
            // Convert the difference (which is usually small relative to the screen) to double/float
            texX = (float)diffX.get_d();
            texY = (float)diffY.get_d();
        } else {
            texX = (float)(renderOffset.x - offset.x);
            texY = (float)(renderOffset.y - offset.y);
        }

        Vector2 texturePos = { texX, texY };
        
        DrawTextureRec(mandelbrotTexture.texture, (Rectangle){0, 0, (float)mandelbrotTexture.texture.width, (float)-mandelbrotTexture.texture.height}, texturePos, WHITE);
        
        if (movingRightNow) {
            long double scale;
            if (usingArbitraryPrecisionLibrary) {
                 mpf_class s = arbOldZoomFactor / arbZoomFactor;
                 scale = s.get_d();
            } else {
                 scale = oldZoomFactor / zoomFactor;
            }


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

        if (IsKeyPressed(KEY_I)) {
            toggledMoreInfo = !toggledMoreInfo;
        }
        if (toggledMoreInfo) {
            moreInfoOffset += (0 - moreInfoOffset) / 10;
        } else {
            moreInfoOffset += (-320 - moreInfoOffset) / 10;
        }

        // MORE INFO display
        Rectangle moreInfoBox = { 5 + moreInfoOffset, 5, 300, 400 };
        DrawRectangleRounded(moreInfoBox, 0.00f, 4, Fade(BLACK, 0.8f));
        DrawRectangleLinesEx(moreInfoBox, 1, Fade(WHITE, 0.5f));

        Rectangle collapseButton = {304 + moreInfoOffset, 15, 30, 50};
        DrawRectangleRec(collapseButton, Fade(BLACK, 0.8f));
        DrawRectangleLinesEx(collapseButton, 1, Fade(WHITE, 0.5f));
        if (CheckCollisionPointRec(GetMousePosition(), collapseButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            editingZoomSpeed = true;
            editingDetail = false;
            editingThreadCount = false;
            toggledMoreInfo = !toggledMoreInfo;
        }


        


        DrawFPS(15 + moreInfoOffset, 15);
        
        if (usingArbitraryPrecisionLibrary) {
            std::stringstream ss; 
            ss << arbZoomFactor;
            std::string z = ss.str().substr(0, 15);
            
            ss.str(""); ss << (arbOffset.x / arbZoomFactor);
            std::string x = ss.str().substr(0, 15);
            
            ss.str(""); ss << (arbOffset.y / arbZoomFactor);
            std::string y = ss.str().substr(0, 15);

            DrawText(TextFormat("Zoom: %s...", z.c_str()), 15 + moreInfoOffset, 35, 20, WHITE);
            DrawText(TextFormat("X Pos: %s...", x.c_str()), 15 + moreInfoOffset, 55, 20, WHITE);
            DrawText(TextFormat("Y Pos: %s...", y.c_str()), 15 + moreInfoOffset, 75, 20, WHITE);
        } else {
            DrawText(TextFormat("Zoom: %s", truncateZeroes(zoomFactor, 15).c_str()), 15 + moreInfoOffset, 35, 20, WHITE);
            DrawText(TextFormat("X Pos: %s", truncateZeroes(offset.x / zoomFactor, 10).c_str()), 15 + moreInfoOffset, 55, 20, WHITE);
            DrawText(TextFormat("Y Pos: %s", truncateZeroes(offset.y / zoomFactor, 10).c_str()), 15 + moreInfoOffset, 75, 20, WHITE);
        }

        int lengthInput;
        // Textboxes and checkboxes options
        // if statement is necessary because the textboxes have a lot of code that i don'T want to see all the time
        if (true) {

            // Iteration amount input area
            Rectangle detailInputBox = { 15 + moreInfoOffset, 100, 100, 30 };
            if (editingDetail) {
                DrawRectangleRec(detailInputBox, Fade(WHITE, 0.3f));
            } else {
                DrawRectangleRec(detailInputBox, Fade(WHITE, 0.1f));
            }
            DrawRectangleLinesEx(detailInputBox, 1, Fade(WHITE, 0.5f));
            DrawText(detailInputText, 20 + moreInfoOffset, 105, 20, WHITE);
            DrawText("Iterations", 125 + moreInfoOffset, 105, 20, WHITE);

            if (CheckCollisionPointRec(GetMousePosition(), detailInputBox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                editingDetail = true;
                editingZoomSpeed = false;
                editingThreadCount = false;
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
                    if (detailAmt < 10) {
                        detailAmt = 10;
                    }
                    needsRedraw = true;
                    editingDetail = false;
                }
            }

            // Editing zoom speed
            Rectangle zoomSpeedInputBox = { 15 + moreInfoOffset, 135, 100, 30 };
            if (editingZoomSpeed) {
                DrawRectangleRec(zoomSpeedInputBox, Fade(WHITE, 0.3f));
            } else {
                DrawRectangleRec(zoomSpeedInputBox, Fade(WHITE, 0.1f));
            }
            DrawRectangleLinesEx(zoomSpeedInputBox, 1, Fade(WHITE, 0.5f));
            DrawText(zoomSpeedInputText, 20 + moreInfoOffset, 140, 20, WHITE);
            DrawText("Zoom speed", 125 + moreInfoOffset, 140, 20, WHITE);

            if (CheckCollisionPointRec(GetMousePosition(), zoomSpeedInputBox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                editingZoomSpeed = true;
                editingDetail = false;
                editingThreadCount = false;
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
                    if (zoomSpeed < 5) {
                        zoomSpeed = 5;
                    }
                    if (zoomSpeed > 200) {
                        zoomSpeed = 200;
                    }
                    // This seems kind of strage but turning up zoomspeed actually makes it slower for some reason lol but this is more intuitive
                    zoomSpeed = (200 / zoomSpeed) * 5 * 2.5f;
                    editingZoomSpeed = false;
                }
            }


            Rectangle arbitraryPrecisionLibraryBox = {15 + moreInfoOffset, 170, 30, 30};
            if (usingArbitraryPrecisionLibrary) {
                DrawRectangleRec(arbitraryPrecisionLibraryBox, Fade(WHITE, 0.3f));
            } else {
                DrawRectangleRec(arbitraryPrecisionLibraryBox, Fade(WHITE, 0.1f));
            }
            DrawRectangleLinesEx(arbitraryPrecisionLibraryBox, 1, Fade(WHITE, 0.5f));
            DrawText("Arbitrary Precision", 55 + moreInfoOffset, 175, 20, WHITE);
            if (CheckCollisionPointRec(GetMousePosition(), arbitraryPrecisionLibraryBox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                usingArbitraryPrecisionLibrary = !usingArbitraryPrecisionLibrary;
                editingZoomSpeed = false;
                editingDetail = false;
                editingThreadCount = false;
                needsRedraw = true; // Force redraw on switch
            }


            Rectangle boxZoomCheckbox = {15 + moreInfoOffset, 205, 30, 30};
            if (usingBoxZoom) {
                DrawRectangleRec(boxZoomCheckbox, Fade(WHITE, 0.3f));
            } else {
                DrawRectangleRec(boxZoomCheckbox, Fade(WHITE, 0.1f));
            }
            DrawRectangleLinesEx(boxZoomCheckbox, 1, Fade(WHITE, 0.5f));
            DrawText("Use box zoom", 55 + moreInfoOffset, 210, 20, WHITE);
            if (CheckCollisionPointRec(GetMousePosition(), boxZoomCheckbox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                usingBoxZoom = !usingBoxZoom;
                editingZoomSpeed = false;
                editingDetail = false;
                editingThreadCount = false;
            }


            Rectangle threadCountInputBox = { 15 + moreInfoOffset, 240, 100, 30 };
            if (editingThreadCount) {
                DrawRectangleRec(threadCountInputBox, Fade(WHITE, 0.3f));
            } else {
                DrawRectangleRec(threadCountInputBox, Fade(WHITE, 0.1f));
            }
            DrawRectangleLinesEx(threadCountInputBox, 1, Fade(WHITE, 0.5f));
            DrawText(threadCountInputText, 20 + moreInfoOffset, 245, 20, WHITE);
            DrawText("Threads", 125 + moreInfoOffset, 245, 20, WHITE);
            if (CheckCollisionPointRec(GetMousePosition(), threadCountInputBox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                editingThreadCount = true;
                editingDetail = false;
                editingZoomSpeed = false;
            }
            if (editingThreadCount) {
                int key = GetCharPressed();
                lengthInput = strlen(threadCountInputText);
                if (key >= '0' && key <= '9' && lengthInput < 5) {
                    threadCountInputText[lengthInput] = (char)key;
                    threadCountInputText[lengthInput + 1] = '\0';
                    
                }
                if (IsKeyPressed(KEY_BACKSPACE) && lengthInput > 0) {
                    threadCountInputText[lengthInput - 1] = '\0';
                }
                if (IsKeyPressed(KEY_ENTER)) {
                    int newThreadCount = atoi(threadCountInputText);
                    if (newThreadCount < 1) {
                        newThreadCount = 1;
                    }
                    if (newThreadCount > 32) {
                        newThreadCount = 32;
                    }

                    threadz = newThreadCount;
                    editingThreadCount = false;
                    needsRedraw = true;
                }
            }

            DrawText("Detail level", 20 + moreInfoOffset, 270, 20, WHITE);
            std::vector<Rectangle> detailLevelBoxes = {
                {15 + moreInfoOffset, 295, 30, 30},
                {50 + moreInfoOffset, 295, 30, 30},
                {85 + moreInfoOffset, 295, 30, 30},
                {120 + moreInfoOffset, 295, 30, 30},
                {155 + moreInfoOffset, 295, 30, 30}
            };
            for (int i = 0; i < detailLevelBoxes.size(); i++) {
                if (detailLevelUsed == i) {
                    DrawRectangleRec(detailLevelBoxes[i], Fade(WHITE, 0.3f));
                } else {
                    DrawRectangleRec(detailLevelBoxes[i], Fade(WHITE, 0.1f));
                }
                DrawRectangleLinesEx(detailLevelBoxes[i], 1, Fade(WHITE, 0.5f));
            }


        }
        

        EndDrawing();
    }
    
    UnloadRenderTexture(mandelbrotTexture);
    CloseWindow();
    return 0;
}