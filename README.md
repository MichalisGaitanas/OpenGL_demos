# OpenGL_demos

# This project contains some examples-tutorials-demos of OpenGL. The codes are written in C++. In order to be able to run all the example codes, the
# following dependencies must be installed : OpenGL kernel (usually preinstalled), glfw, glew, glm, freetype and a C++ compiler.



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
g++ program.cpp -o program.exe (PATH_TO_HERE)\imgui\imgui*.cpp -I (PATH_TO_HERE)\imgui [... OpenGL libs that were mentioned above ...]

If the 'OpenGL_demos/imgui/' contains implot cpp codes, then :

g++ program.cpp -o program.exe (PATH_TO_HERE)\imgui\imgui*.cpp (PATH_TO_HERE)\imgui\implot*.cpp -I (PATH_TO_HERE)\imgui [... OpenGL libs that were mentioned above ...]

Compile as a static library :
a) g++ -c *.cpp
b) ar rcs libimgui.a *o
Library is created. Then : g++ programm.cpp -o program.exe -L path/to/imgui -limgui [... OpenGL libs that were mentioned above ...]

# 5)
In order to follow the examples, it is essential that you know a bit of C++. We recommend this website [ https://www.learncpp.com ]

# 6)
More OpenGL tutorials can be found in this website [ https://learnopengl.com ]