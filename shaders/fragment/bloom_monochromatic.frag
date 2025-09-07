#version 450 core

out vec4 frag_col;



uniform vec3 mesh_col; //Mesh color.
uniform float bloom_intensity; //>= 1.0.

void main()
{
    frag_col = vec4(mesh_col*bloom_intensity, 1.0f); //Same color for all fragments.
}