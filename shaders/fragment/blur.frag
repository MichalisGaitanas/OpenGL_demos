#version 330 core

in vec2 uv;
out vec4 frag_col;

uniform sampler2D sample_tex;

void main()
{
    //Instead of directly sampling the texture bound to texture unit 0 at the specified u,v coordinates
    //and return the color of the corresponding texel (pixel in the texture), we will 'play' around with
    //the uv coords: We will sample from slightly distant (offset) uv coords.

    float offset = 1.0f/500.0f; //Play with this value for more/less blur.

    vec3 result = texture(sample_tex, uv).rgb*0.227027f; //Center weight.
    
    result += texture(sample_tex, uv + vec2(-offset, 0.0f)).rgb*0.1945946f;
    result += texture(sample_tex, uv + vec2( offset, 0.0f)).rgb*0.1945946f;
    result += texture(sample_tex, uv + vec2(-2.0f*offset, 0.0f)).rgb*0.1216216f;
    result += texture(sample_tex, uv + vec2( 2.0f*offset, 0.0f)).rgb*0.1216216f;
    result += texture(sample_tex, uv + vec2(-3.0f*offset, 0.0f)).rgb*0.054054f;
    result += texture(sample_tex, uv + vec2( 3.0f*offset, 0.0f)).rgb*0.054054f;

    frag_col = vec4(result, 1.0f);
}