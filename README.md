# OpenGL_demos

This project contains some demos-examples-tutorials of OpenGL. The codes are written in C++.

In order to follow the demos, it is essential that you know some C++. We recommend this website [https://www.learncpp.com].
OpenGL tutorials can be found in this website [https://learnopengl.com].

In order to to run all the demo codes, the following dependencies must be installed :
1) C++ compiler. We use GNU's gcc/g++, provided by MSYS2 MinGW-w64
2) OpenGL kernel. This is usually preinstalled in all operating systems.
3) GLFW. This provides hardware functionality (keyboard, mouse, monitor, Xbox controller, etc...).
4) GLEW. We need GLEW because OpenGL function pointers, especially for modern features, aren't loaded automatically by the system. GLEW
   dynamically loads these functions at runtime, allowing your application to use modern OpenGL across different hardware and drivers.
5) GLM. Fast algebra-geometry functions needed for spatial rendering and transformations (dot product, cross product, matrix multiplication, etc...).

In case we want to add GUI rendering and plots (see below), ImGui and ImPlot must be present as well. Here, the corresponding libraries are already present
in the /imgui directory. Originally obtained via : [https://github.com/ocornut/imgui] and [https://github.com/epezent/implot].

-------------------------------------------------------------------------------------------------------------------------------------------------

Compile and run the .cpp programs in the /demos directory (Windows) :

1) Using CMake :

On Windows, install MSYS2.

From the "MSYS2 MinGW x64" shell install the essential :

pacman -Syu
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake mingw-w64-x86_64-glew mingw-w64-x86_64-glfw mingw-w64-x86_64-glm

Clone the repo :

git clone https://github.com/MichaelGaitanas/OpenGL_demos
cd /.../OpenGL_demos

Create a build directory :

mkdir build
cd build

Run CMake to configure the project. In the MSYS2 MinGW64 shell :

cmake -G "MinGW Makefiles" ..
cmake --build .

After this completes, you will have executable (.exe) files for each demo in the build directory.





2) Using manual g++ commands :

'Add to path' the g++ compiler, so you can execute commands in the cmd/powershell.

Assuming that the demo does NOT use ImGui/ImPlot stuff, the command is :

In cmd : g++ demo.cpp -o demo.exe -lopengl32 -lglfw3 -lglew32

Assuming that the demo uses ImGui/ImPlot stuff, the command is :

In cmd : g++ demo.cpp -o demo.exe (PATH_TO_OPENGL_DEMOS)\imgui\*.cpp -I (PATH_TO_OPENGL_DEMOS)\imgui -lopengl32 -lglfw3 -lglew32

It is a good idea to (pre)compile it as a static library, so that you don't have to compile every time the same code.

Compile as a static library : Navigate to the imgui folder and then, in the cmd write :
a) g++ -c *.cpp
b) ar rcs libimgui.a *.o
Library is created.
Then, for every new code : g++ demo.cpp -o demo.exe -L (PATH_TO_OPENGL_DEMOS)\imgui -limgui -lopengl32 -lglfw3 -lglew32

Be strict with the compiler and add the flags -Wall -Wpedantic -Wextra -Werror

One can also run the compile_all.bat to compile all .cpp codes and generate the executables.

-------------------------------------------------------------------------------------------------------------------------------------------------