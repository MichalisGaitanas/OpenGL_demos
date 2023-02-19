#version 330 core

in vec2 tex_coords;
out vec4 frag_col;

uniform sampler2D text;
uniform vec3 text_col;

void main()
{    
    vec4 sampled = vec4(1.0f,1.0f,1.0f, texture(text, tex_coords).r);
    frag_col = vec4(text_col, 1.0f)*sampled;
}