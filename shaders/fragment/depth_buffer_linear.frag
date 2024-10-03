#version 330 core

out vec4 frag_col;


void main()
{
    float depth = gl_FragCoord.z; //Default depth value that ranges in [0,1].

    depth = 2.0f*depth - 1.0f; //Depth value, mapped in range [-1,1] (1st transformation).

    float near = 0.1f;
    float far = 100.0f;
    depth = (2.0f*near*far)/(far + near - depth*(far - near)); //Depth value, mapped in range [near,far] (2nd transformation).
    depth /= far; //Depth value, mapped in range [0,1] (3rd transformation).

    frag_col = vec4(depth, depth, depth, 1.0f);
}