# OpenGL_demos

# This project contains some examples-tutorials-demos of the OpenGL C API. The codes however, are written in C++.
# In order to be able to run all the example codes, the following dependencies must be installed : OpenGL kernel, glfw, glew, glm, freetype and a C++ compiler.



# Useful advice :

# 1)
#include<GL/glew.h> //first include GL/glew.h before any other OpenGL header
#include<GLFW/glfw3.h>

# 2)
//do not forget this piece of code before the rendering loop
glewExperimental = GL_TRUE;
if (glewInit() != GLEW_OK)
{
    printf("Failed to initialize GLEW. Exiting...\n");
    return -1;
}

# 3)
Windows, cmd (assuming the installations : g++ (add to path), glfw, glew)
g++ program.cpp -o program.exe -lopengl32 -lglfw3 -lglew32
g++ program.cpp -o program.exe -lopengl32 -lglfw3 -lglew32 -lfreetype (if freetype is utilized)

Linux, terminal (assuming the installations : glfw, glew)
g++ program.cpp -o program -lGL -lglfw -lGLEW
g++ program.cpp -o program -lGL -lglfw -lGLEW -lfreetype (if freetype is utilized)

Note : In case of font rendering, the ft2build.h must be in the default c++ include directory.
Also only 1 freetype/ directory should exist. So if freetype is installed like freetype2/freetype/headers.h,
remove the freetype2/.

# 4)
imgui and glfw :

//first include imgui stuff
#include"imgui.h"
#include"imgui_impl_glfw.h"
#include"imgui_impl_opengl3.h"

//then these
#include<GL/glew.h>
#include<GLFW/glfw3.h>

Assuming there is an 'OpenGL_demos/imgui/' folder,
g++ program.cpp -o program.exe C:\Users\micha\github\OpenGL_demos\imgui\imgui*.cpp -I C:\Users\micha\github\OpenGL_demos\imgui [... OpenGL libs ...]

# 5)
In order to follow the examples, it is essential that you know a bit of C++. We recommend this website [ https://www.learncpp.com ]

# 6)
More OpenGL tutorials can be found in this website [ https://learnopengl.com ]





# Project structure :

The folder shaders/ contains all the vertex and fragment shaders, written in glsl ([g]raphics [l]ibrary [s]hading [s]anguage).

The folder obj/ contains all the .obj files (models), that are loaded and rendered in the examples.
'/vf/'  means that the .obj files inside contain only [v]ertex coordinates and [f]ace indices.
'/vfn/' means that the .obj files inside contain [v]ertex coordinates, [f]ace indices and [n]ormal vectors (either per vertex or per face).
'/vft/' means that the .obj files inside contain [v]ertex coordinates, [f]ace indices and [t]exture coordinates.

The include/ folder contains classes and functions that clarify the code.

The folder imgui/ is basically a bunch of .h and .cpp files that utilizes native OpenGL functions (like those in our examples), in
order to render [i]mmediate [m]ode [g]raphical [u]ser [i]nterface.

The images/ folder contains images that can be used as skyboxes (cubemaps), textures, etc...

The fonts/ folder contains only .ttf files that are used in case of font rendering.

The examples/ folder assembles all the aforementioned to create graphics in ours screen.