// Improvements made:
// -----------------------
// Logarithmic zoom
// Moving entire texture instead of recalculating
// Float -> Double
// Smart redrawing
// Double -> long double
// Local coordinates

// Optimizations / features to make:
// Loading screen so that you dont move during calculations
// Shading
// Add a number line, mouse pointer coord display
// Paramaterize z, make Julia set

#include <math.h>
#include <cstdlib>
#include <complex>
#include <vector>

#include "raylib.h"
#include "raymath.h"



struct SuperVector2 {
    long double x;
    long double y;
};

bool canMove = true;
long double zoomFactor = 250;

bool usingHeatMapOrNo = false;

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
    // These early breaks might be helpful for bigger pictures

    std::complex<long double> z_old(0.0L, 0.0L);
    int period = 0;
    for (int i = 0; i < 100; i++)
    {
        z = z * z + c;
        if (std::abs(z) > 2.0L)
        {
            return false;
        }
        if (i == period)
        {
            if (std::abs(z - z_old) < 1e-10L)
            {
                return true;
            }
            z_old = z;
            period = i;
        }
    }
    return true;
}

int isInMandlebrotButGiveIterationsToEscape(std::complex<long double> c) {
    std::complex<long double> z(0.0L, 0.0L);
    std::complex<long double> z_old(0.0L, 0.0L);
    int period = 0;
    for (int i = 0; i < 100; i++)
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

    if (usingHeatMapOrNo) {
        // If the heat map is on, then use the isInMandlebrotButGiveIterationsToEscape function, not implemented yet but this is how i want to do it:
        // Go through each pixel, just like the nested for loops below

        
    } else {

        for (int x = setWidth * -0.5; x <= setWidth * 0.5; x++) {
            for (int y = setHeight * -0.5; y <= setHeight * 0.5; y++) {
                long double real = centerReal + x / zoomFactor;
                long double imag = centerImag + y / zoomFactor;

                if (isInMandelbrot(std::complex<long double>(real, imag))) {
                    DrawRectangle(setWidth / 2 + x, setHeight / 2 + y, 1, 1, RED);
                }


            }
        }

    }

    EndTextureMode();
}

SuperVector2 offsetControls(long double offsetX, long double offsetY, RenderTexture2D mandelbrotTexture, SuperVector2 *renderOffset, bool *needsRedraw) {
    SuperVector2 returnVal = {offsetX, offsetY};
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
        returnVal.y = offsetY + 1.0L;
    }
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
        returnVal.y = offsetY - 1.0L;
    }
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        returnVal.x = offsetX - 1.0L;
    }
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        returnVal.x = offsetX + 1.0L;
    }

    // Check if any movement key was released
    if (IsKeyReleased(KEY_DOWN) || IsKeyReleased(KEY_S) ||
        IsKeyReleased(KEY_UP) || IsKeyReleased(KEY_W) ||
        IsKeyReleased(KEY_LEFT) || IsKeyReleased(KEY_A) ||
        IsKeyReleased(KEY_RIGHT) || IsKeyReleased(KEY_D))
    {
        *needsRedraw = true;
    }

    return returnVal;
}

int main(void) {
    SuperVector2 offset = {0, 0};
    SuperVector2 renderOffset = {0, 0};
    InitWindow(600, 600, "Mandelbrot");

    RenderTexture2D mandelbrotTexture = LoadRenderTexture(600, 600);
    bool needsRedraw = true;

    SetTargetFPS(120);

    long double previousZoom = zoomFactor;

    while (!WindowShouldClose()) {
        if (canMove) {
            SuperVector2 newOffset = offsetControls(offset.x, offset.y, mandelbrotTexture, &renderOffset, &needsRedraw);
            offset = newOffset;

            if (IsKeyDown(KEY_M)) {
                zoomFactor += zoomFactor / 50.0L;
            }
            if (IsKeyDown(KEY_N)) {
                zoomFactor -= zoomFactor / 50.0L;
            }
            if (IsKeyReleased(KEY_M) || IsKeyReleased(KEY_N)) {
                if (fabsl(zoomFactor - previousZoom) > 0.01L) {
                    long double zoomRatio = zoomFactor / previousZoom;
                    offset.x = offset.x * zoomRatio;
                    offset.y = offset.y * zoomRatio;

                    needsRedraw = true;
                    previousZoom = zoomFactor;
                }
            }
        }

        if (needsRedraw) {
            canMove = false;
            renderMandelbrotToTexture(mandelbrotTexture, offset.x, offset.y);
            renderOffset = offset;
            needsRedraw = false;
            canMove = true;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        Vector2 texturePos = { (float)(renderOffset.x - offset.x), (float)(renderOffset.y - offset.y) };
        DrawTextureRec(mandelbrotTexture.texture,
                        (Rectangle){0, 0, (float)mandelbrotTexture.texture.width, (float)-mandelbrotTexture.texture.height},
                        texturePos, WHITE);

        drawIntercepts(offset.x, offset.y);
        DrawFPS(10, 10);
        DrawText(TextFormat("Zoom: %.1Lf", zoomFactor), 10, 30, 20, DARKGRAY);

        EndDrawing();
    }

    UnloadRenderTexture(mandelbrotTexture);
    CloseWindow();
    return 0;
}

// listening to kid cudi rightnow he's peak
// My top 3 kanye (objectively correct):
// 1. Ghost town
// 2. Family Business
// 3. Waves
