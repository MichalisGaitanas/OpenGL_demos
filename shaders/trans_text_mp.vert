#version 330 core
layout (location = 0) in vec4 data; //<vec2 pos, vec2 tex>

out vec2 texCoords;
uniform mat4 projection;
uniform mat4 model;

void main()
{
    vec2 pos = data.xy;
    vec2 tex = data.zw;
    gl_Position = projection*model*vec4(pos,0.0f,1.0f);
    texCoords = tex;
}