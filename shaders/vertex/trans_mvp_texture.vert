#version 450 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 tex;

out vec2 uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection*view*model*vec4(pos,1.0f);
    uv = tex;
}