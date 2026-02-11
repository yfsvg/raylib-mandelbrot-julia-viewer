### 2 things you HAVE to know RIGHT NOW RAHHH IF YOU ONLY HAVE THE PATIENCE TO READ ONE SECTION IT BETTER BE THIS ONE
1. If you are using the web viewer, PLEASE be patient and wait for the initial image to generate please please! wasm is far slower than native c++ so be patient thank you <3
2. You will enter "movement mode" once you start pressing WASD or M/N for zooming in. The MOMENT you release all of the keys, frames are going to start rendering and you won't be able to move.



***

# Mandelbrot/Julia Set Viewer

Raylib viewer to see the Mandelbrot set and also the lesser known Julia set family. Very pretty fractals and stuff. The math is incredibly simple, in essence it just sees if a complex point diverges in a sequence and it makes this mess for some reason but it looks pretty. 

***

# How to access

There is a website that I will be pasting a link to in here very soon. In the meantime it is also possible to download it directly together with raylib, which is the better option if you want to use the arbitrary position library.

# Arbitrary precision

After zoom exceeds ##e+15, you start seeing precision issues because the regular viewer uses long double to hold position variables. There is an option to use an **arbitrary precision library** but it is not available in the website because I dont feel like setting up gmp in emscripten actually it might be possible stay tuned i mgiht add it.

## How to use

Upon entering the visualizer, controls are automatically set to **using WASD to move the frame up and down and M to zoom in and N to zoom out**. If you unselect "Using WASD" you can use your mouse to drag around the screen. The frame will recalculate stuff and refresh when you stop moving.

These sets are infinitely complex, but its not possible to show all of the detail at the same time. Change the number next to "Iterations" to increase the detail of the set, allowing you to see more interesting details. Moar iterations = moar detail and better zoom

It can get slow when very zoomed in or using arbitrary position libraries. Change the amount of threads being used to increase performance, or use low detail mode or ultra low detail mode to increase pixel size and therefore reduce calculation and rendering time.

***

My discord is @yefoi dm me there for questions 🙀
