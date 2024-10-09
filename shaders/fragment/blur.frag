#version 330 core

in vec2 tex_coords;
out vec4 frag_col;

uniform sampler2D tex;

void main()
{
    float offset = 1.0f/500.0f; //Change this value for more/less blur.
    vec3 result = texture(tex, tex_coords).rgb*0.227027f; //Center weight.
    result += texture(tex, tex_coords + vec2(-offset, 0.0f)).rgb*0.1945946f;
    result += texture(tex, tex_coords + vec2( offset, 0.0f)).rgb*0.1945946f;
    result += texture(tex, tex_coords + vec2(-2.0f*offset, 0.0f)).rgb*0.1216216f;
    result += texture(tex, tex_coords + vec2( 2.0f*offset, 0.0f)).rgb*0.1216216f;
    result += texture(tex, tex_coords + vec2(-3.0f*offset, 0.0f)).rgb*0.054054f;
    result += texture(tex, tex_coords + vec2( 3.0f*offset, 0.0f)).rgb*0.054054f;

    frag_col = vec4(result, 1.0f);
}