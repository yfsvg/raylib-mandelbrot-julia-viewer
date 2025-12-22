// The mandelbrot project begins :)

#include "raylib.h"
#include "raymath.h"

int main(void)
{
    InitWindow(600, 600, "Mandelbrot Set Viewer");
    SetTargetFPS(120);
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}