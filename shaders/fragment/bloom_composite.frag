#version 450 core
layout(location=0) out vec4 frag_col;
in vec2 uv;

uniform sampler2D scene;        // HDR scene
uniform sampler2D bloom;        // blurred highlights
uniform float     bloom_strength; // 0..1+

// ACES (Narkowicz 2015) approximation
vec3 ACES(vec3 x)
{
    const float a=2.51,b=0.03,c=2.43,d=0.59,e=0.14;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

void main()
{
    vec3 hdr   = texture(scene, uv).rgb;
    vec3 bloom = texture(bloom, uv).rgb;
    vec3 color = hdr + bloom_strength * bloom; // additive in HDR
    color = ACES(color);                        // tonemap to [0,1]
    color = pow(color, vec3(1.0/2.2));         // gamma → sRGB
    frag_col = vec4(color, 1.0);
}