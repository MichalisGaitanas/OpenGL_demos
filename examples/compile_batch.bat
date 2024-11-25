@echo off
REM set FILES=ex1_black_window.cpp ex2_colored_window.cpp

for %%f in (*.cpp) do (
    g++ %%f -o %%~nf.exe -L C:\Users\Michalis\github\OpenGL_demos\imgui -limgui -lopengl32 -lglew32 -lglfw3
)