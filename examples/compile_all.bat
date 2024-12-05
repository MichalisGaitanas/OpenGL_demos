@echo off
REM set FILES=ex1_black_window.cpp ex2_colored_window.cpp, etc... and then : for %%f in (%FILES%) do (
for %%f in (*.cpp) do (
    g++ %%f -o %%~nf.exe -L "%USERPROFILE%\github\OpenGL_demos\imgui" -limgui -lopengl32 -lglew32 -lglfw3 -Wall -Wpedantic -Wextra -Werror
)