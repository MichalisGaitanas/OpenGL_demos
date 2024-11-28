//This fragment shader is only meant for rendering asteroid lightcurves.

#version 450 core

in vec3 frag_pos_world;
in vec4 frag_pos_light;
in vec3 normal;

out vec4 frag_col; //Final color of the fragment after lighting calculations.

uniform vec3 light_dir; //Direction of the light in world coordinates.
uniform sampler2D sample_shadow; //Depth image texture, obtained by the other shader.

//Algorithm to decide whether the fragment is in shadow or not.
float get_shadow(vec3 norm, vec3 light_dir_norm)
{
    vec3 projected_coords = frag_pos_light.xyz/frag_pos_light.w; //Perspective division to transform each fragment's position (with respect to light) in NDC, i.e. in [-1, 1].
    projected_coords = 0.5f*projected_coords + vec3(0.5f); //Transformation from [-1, 1] to [0, 1]. This is required to correctly access the shadow map texture, because internally, the UVs range in [0, 1].
    
    //For any fragment that is outside the orthographic frustum, don't calculate shadow.
    if (projected_coords.x < 0.0f || projected_coords.x > 1.0f ||
        projected_coords.y < 0.0f || projected_coords.y > 1.0f ||
        projected_coords.z > 1.0f)
    {
        return 0.0f; //No shadow. Fully lit.
    }

    //Shadow acne fix : Balance between shadow acne (self-shadowing) and Peter-panning.
    float min_bias = 0.0006f, amplifier = 0.006f;
    float bias = max(amplifier*(1.0f - max(dot(norm, light_dir_norm), 0.0f)), min_bias);

    float nearest_frag_depth = texture(sample_shadow, projected_coords.xy).r; //Sample depth from the shadow map.
    if (projected_coords.z - bias > nearest_frag_depth)
        return 1.0f; //In shadow.
    return 0.0f; //Not in shadow.
}

void main()
{
    vec3 norm = normalize(normal);
    vec3 light_dir_norm = normalize(light_dir);
    float diffuse = max(dot(norm, light_dir_norm), 0.0f);
    float shadow = get_shadow(norm, light_dir_norm);
    frag_col = vec4(vec3((1.0f - shadow)*diffuse), 1.0f);
}
