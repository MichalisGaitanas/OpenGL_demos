#version 450 core

layout(location=0) out vec4 frag_col;

in vec2 uv;

uniform sampler2D image;
uniform bool      horizontal;
uniform float bloom_radius;

void main() {
    ivec2 ts = textureSize(image, 0);
    vec2 texel = 1.0 / vec2(ts);

    float w[5] = float[5](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

    vec3 c = texture(image, uv).rgb * w[0];
    for (int i=1; i<5; ++i)
    {
        vec2 off = (horizontal ? vec2(texel.x*i, 0.0) : vec2(0.0, texel.y*i))*bloom_radius;
        c += texture(image, uv + off).rgb * w[i];
        c += texture(image, uv - off).rgb * w[i];
    }
    frag_col = vec4(c, 1.0);
}