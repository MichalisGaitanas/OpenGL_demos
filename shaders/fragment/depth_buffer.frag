#version 330 core

out vec4 frag_col;

void main()
{
    frag_col = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.0f);
    //FragCoord.z is basically the stored depth value of the current fragment.
}