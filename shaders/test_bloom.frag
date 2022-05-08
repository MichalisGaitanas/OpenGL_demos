#version 330 core

in vec3 fragPos;
out vec4 fragCol;

uniform vec3 modelCol;

void main()
{

    float d = length(fragPos);
    fragCol = vec4(10*exp(-5*d)*modelCol, 1.0f);
}
