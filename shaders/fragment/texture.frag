#version 330 core

in vec2 tex_coords;
out vec4 frag_col;

uniform sampler2D tex;

/*
void main()
{
	frag_col = texture(tex, tex_coords);
}
*/

void main()
{
	float offset = 1.0 / 300.0; // Change this value for more/less blur
    vec3 result = texture(tex, tex_coords).rgb * 0.227027; // center weight
    
    result += texture(tex, tex_coords + vec2(-offset,  0.0)).rgb * 0.1945946;
    result += texture(tex, tex_coords + vec2( offset,  0.0)).rgb * 0.1945946;
    result += texture(tex, tex_coords + vec2(-2.0*offset,  0.0)).rgb * 0.1216216;
    result += texture(tex, tex_coords + vec2( 2.0*offset,  0.0)).rgb * 0.1216216;
    result += texture(tex, tex_coords + vec2(-3.0*offset,  0.0)).rgb * 0.054054;
    result += texture(tex, tex_coords + vec2( 3.0*offset,  0.0)).rgb * 0.054054;

	frag_col = vec4(result, 1.0);
}