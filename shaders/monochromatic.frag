#version 330 core

out vec4 fragCol;

uniform vec3 modelCol;

void main()
{
    fragCol = vec4(modelCol, 1.0f); //same color for all fragments
}