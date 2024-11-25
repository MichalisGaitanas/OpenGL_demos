@echo off
REM set FILES=demo1_aster_lightcurve_no_shadow.cpp

for %%f in (*.cpp) do (
    g++ %%f -o %%~nf.exe -L C:\Users\Michalis\github\OpenGL_demos\imgui -limgui -lopengl32 -lglew32 -lglfw3
)