#version 330 core

in vec2 uv;
out vec4 frag_col;

uniform sampler2D sample_tex;

void main()
{
    //Instead of directly sampling the texture bound to texture unit 0 at the specified u,v coordinates
    //and return the color of the corresponding texel (pixel in the texture), we will 'play' around with
    //the uv coords: We will sample from slightly distant (offset) uv coords. The coefficients
    //(0.1945946, 0.1216216f, 0.0540540) reduce the effect of sample contribution (negative and positive)
    //in accordance to a gaussian function, hence the name 'gaussian blur' we see in many softwares.

    float offset = 1.0f/400.0f; //Play with this value for more/less blur.

    vec3 result = texture(sample_tex, uv).rgb*0.227027f; //Center weight.
    
    result += 0.1945946f*texture(sample_tex, uv + vec2(-offset, 0.0f)).rgb;
    result += 0.1945946f*texture(sample_tex, uv + vec2( offset, 0.0f)).rgb;
    result += 0.1216216f*texture(sample_tex, uv + vec2(-2.0f*offset, 0.0f)).rgb;
    result += 0.1216216f*texture(sample_tex, uv + vec2( 2.0f*offset, 0.0f)).rgb;
    result += 0.0540540f*texture(sample_tex, uv + vec2(-3.0f*offset, 0.0f)).rgb;
    result += 0.0540540f*texture(sample_tex, uv + vec2( 3.0f*offset, 0.0f)).rgb;

    frag_col = vec4(result, 1.0f);
}