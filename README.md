# OpenGL_demos

# This project contains some examples-tutorials-demos of OpenGL. The codes are written in C++.

# In order to follow the examples, it is essential that you know a bit of C++. We recommend this website [https://www.learncpp.com].
# OpenGL tutorials can be found in this website [https://learnopengl.com].

# In order to be able to run all the example codes, the following dependencies must be installed :
# 1) C++ compiler. Choose either GNU's gcc (g++) or Microsoft's MSVC.
# 2) OpenGL kernel. This is usually preinstalled in all operating systems.
# 3) GLFW. This provides hardware functionality (keyboard, mouse, monitor, Xbox controller, etc...).
# 4) GLEW. This is complicated... We need GLEW because OpenGL function pointers, especially for modern features, aren't loaded automatically by the system. GLEW
#    dynamically loads these functions at runtime, allowing your application to use modern OpenGL across different hardware and drivers.
# 5) GLM. Insanely fast algebra and geometry functions needed for spatial rendering and transformations (dot product, cross product, matrix multiplication, etc...).

# Note : In case we want to add GUI rendering and plots (see below), ImGui and ImPlot must be installed as well. Here, the corresponding libraries are already present
# in the /imgui directory. Originally obtained. via : [https://github.com/ocornut/imgui] and [https://github.com/epezent/implot].


# Useful advice :

# Advice 1) First include GL/glew.h before any other OpenGL header. In case of ImGui/ImPlot usage, neglect this advice and see below.

#include<GL/glew.h>
#include<GLFW/glfw3.h>
...



# Advice 2) Do not forget this piece of code before the glfw rendering loop.

...
glewExperimental = GL_TRUE;
if (glewInit() != GLEW_OK)
{
    printf("Failed to initialize GLEW. Exiting...\n");
    return -1;
}
...

# Advice 3) Assuming that the code uses ImGui/ImPlot libraries, you must first include the corresponding headers :

#include"imgui.h"
#include"imgui_impl_glfw.h"
#include"imgui_impl_opengl3.h"

#include"../imgui/implot.h" //Add this only if you intend to use ImPlot functions. Otherwise it is not necessary.

//Then comes the following.

#include<GL/glew.h>
#include<GLFW/glfw3.h>


# Advice 4) In case you use g++ compiler, here's how you can compile and run the .cpp programs in the /examples directory :

-------------------------------------------------------------------------------------------------------------------------------------------------

Windows :

Assuming that the example does NOT use ImGui/ImPlot stuff :

In cmd : g++ example.cpp -o example.exe -lopengl32 -lglfw3 -lglew32

Assuming that the example uses ImGui/ImPlot stuff :

In cmd : g++ example.cpp -o example.exe (PATH_TO_OPENGL_DEMOS)\imgui\*.cpp -I (PATH_TO_OPENGL_DEMOS)\imgui -lopengl32 -lglfw3 -lglew32

It is a good idea to (pre)compile it as a static library, so that you don't have to compile every time the same code.

Compile as a static library : Navigate to the imgui folder and then, in the cmd write :
a) g++ -c *.cpp
b) ar rcs libimgui.a *o
Library is created.
Then, for every new code : g++ example.cpp -o example.exe -L (PATH_TO_OPENGL_DEMOS)\imgui -limgui -lopengl32 -lglfw3 -lglew32

-------------------------------------------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------------------------------------------

Linux :

Assuming that the example does NOT use ImGui/ImPlot stuff :

g++ example.cpp -o example -lGL -lglfw -lGLEW

Assuming that the example uses ImGui/ImPlot stuff :

In cmd : g++ example.cpp -o example (PATH_TO_OPENGL_DEMOS)/imgui/*.cpp -I (PATH_TO_OPENGL_DEMOS)/imgui -lGL -lglfw -lGLEW

The steps for precompiling ImGui/ImPlot, are the same with the Windows steps.

Then, for every new code : g++ example.cpp -o example -L (PATH_TO_OPENGL_DEMOS)/imgui -limgui -lGL -lglfw -lGLEW

-------------------------------------------------------------------------------------------------------------------------------------------------
