#version 330 core

layout(location = 0) in vec3 pos;

uniform mat4 dir_light_projection;
uniform mat4 dir_light_view;
uniform mat4 model;

void main()
{
    gl_Position = dir_light_projection*dir_light_view*model*vec4(pos, 1.0f);
}