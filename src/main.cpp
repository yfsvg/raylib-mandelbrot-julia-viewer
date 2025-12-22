// The mandelbrot project begins :)
// Setup raylib, figured out how to actually use github properly lol
// ken lee

// Things to do for this project:
// 1. Making the mandelbrot implementation actually work
// 2. Adding a zooming function, and then somehow making the offset work with that
// 3. Useable ui

#include <math.h>
#include <cstdlib>
#include <complex>
#include "raylib.h"
#include "raymath.h"

double zoomFactor = 250;

bool movingRightNow = false;

void drawOrigin(float offsetX, float offsetY) {
    DrawRectangle((GetScreenWidth() / 2 - 1) - offsetX, 0, 2, GetScreenHeight(), RED);
    DrawRectangle(0, (GetScreenWidth() / 2 - 1) - offsetY, GetScreenWidth(), 2, RED);
}


Vector2 offsetControls(float offsetX, float offsetY) {

    Vector2 returnVal = {offsetX, offsetY};
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
        returnVal.y = offsetY + 1.0;
        movingRightNow = true;
    }
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
        returnVal.y = offsetY - 1.0;
        movingRightNow = true;
    }
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        returnVal.x = offsetX - 1.0;
        movingRightNow = true;
    }
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        returnVal.x = offsetX + 1.0;
        movingRightNow = true;
    }

    return returnVal;
}

bool isInMandelbrot(std::complex<double> c) {
    std::complex<double> z(0.0, 0.0);
    // Detecting if the points are in the main cardiod and the second biggest cardiod, useful when its big
    if ((c.real() + 0.25) * (c.real() + 0.25) + c.imag() * c.imag() < 0.25) {
        return true;
    }
    if ((c.real() + 1) * (c.real() + 1) + c.imag() * c.imag() < 0.06) {
        return true;
    }

    std::complex<double> z_old(0.0, 0.0);
    // Detecting periodocity (If a number loops between two things over and over again then its in the thing)
    int period = 0;
    for (int i = 0; i < 100; i++)
    {
        z = z * z + c;
        if (std::abs(z) > 2.0)
        {
            return false;
        }
        if (i == period)
        {
            if (std::abs(z - z_old) < 1e-10)
            {
                return true;
            }
            z_old = z;
            period = i;
        }
    }
    return true;
}

// This draws out all of the points of mandelbrot set one by one! I sure hope I don't regret this implementation.
// I got a suggestion to use some marching squares thing but that sounds too complicated lololol i'll make it smooth later
void drawAllPoints(float offsetX, float offsetY) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int centerX = screenWidth / 2;
    int centerY = screenHeight / 2;

    for (int x = screenWidth * -0.5; x <= screenWidth * 0.5; x++) {
        for (int y = screenHeight * -0.5; y <= screenHeight * 0.5; y++) {

            if (isInMandelbrot(std::complex<double>((x + offsetX) / zoomFactor, (y + offsetY) / zoomFactor))) {
                DrawRectangle(centerX + x, centerY + y, 1, 1, RED);
            }

        }
    }
}


int main(void)
{
    Vector2 offset = {0, 0};
    InitWindow(600, 600, "Mandelbrot Set Viewer");
    SetTargetFPS(120);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        offset = offsetControls(offset.x, offset.y);
        drawAllPoints(offset.x, offset.y);
        if (IsKeyDown(KEY_M))
        {
            zoomFactor += 0.1;
        }
        if (IsKeyDown(KEY_N))
        {
            zoomFactor -= 0.1;
        }
        
        drawOrigin(offset.x, offset.y);

        
        EndDrawing();
    }

    CloseWindow();
    return 0;
}


// ohhhh Im watching the new Lemino video, somehow I didn't see it until now :(
// It's the one about the pioneer