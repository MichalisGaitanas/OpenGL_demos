#version 450 core

layout(location = 0) out vec4 frag_col;

in vec2 uv;

uniform sampler2D scene;
uniform float     bloom_threshold;
uniform float     bloom_knee;

float luminance(vec3 c)
{
    return dot(c, vec3(0.2126, 0.7152, 0.0722));
}

void main()
{
    vec3  c  = texture(scene, uv).rgb;
    float l  = luminance(c);
    float x  = max(l - bloom_threshold, 0.0);
    float soft = (bloom_knee > 0.0) ? (x*x) / (bloom_knee + x) : step(bloom_threshold, l) * x;
    float scale = (l > 1e-6) ? soft / l : 0.0;

    vec3 outC = c * clamp(scale, 0.0, 1.0);
    frag_col = vec4(outC, 1.0);
}