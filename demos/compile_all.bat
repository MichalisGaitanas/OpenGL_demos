@echo off
REM set FILES=demo1....cpp demo2....cpp, etc... and then : for %%f in (%FILES%) do (
for %%f in (*.cpp) do (
    g++ %%f -o %%~nf.exe -L "%USERPROFILE%\github\OpenGL_demos\imgui" -limgui -lopengl32 -lglew32 -lglfw3
)