#version 330 core

in vec3 frag_pos_cam;
in vec4 frag_pos_light;
in vec3 normal;

out vec4 frag_col; //Final color of the fragment after lighting calculations.



uniform vec3 mesh_col; //Mesh color.
uniform vec3 light_dir; //Direction of the light in world coordinates.
uniform vec3 light_col; //Light color.
uniform sampler2D sample_shadow; //Depth image texture, obtained by the other shader.

//Algorithm to decide whether the fragment is in shadow or not.
float get_shadow(vec4 frag_pos_light, vec3 norm, vec3 light_dir_norm)
{
    vec3 projected_coords = frag_pos_light.xyz/frag_pos_light.w; //Perspective division to obtain NDC in [-1,1] for each component.
    projected_coords = 0.5f*projected_coords + 0.5f; //Transform to [0,1]. This is required to access the shadow map texture, because internally, the range is in [0,1].
    //For any fragment that is outside the orthographic frustum, don't calculate shadow.
    if (projected_coords.x < 0.0f || projected_coords.x > 1.0f ||
        projected_coords.y < 0.0f || projected_coords.x > 1.0f ||
        projected_coords.z > 1.0f)
    {
        return 0.0f;
    }

    //Read the shadow map's depth value, which corresponds to the nearest fragment (red channel only coz the map has grayscale values only).
    float nearest_frag_depth = texture(sample_shadow, projected_coords.xy).r;
    float current_frag_depth = projected_coords.z;

    //if (current_frag_depth > nearest_frag_depth)
    //  return 1.0f;
    //return 0.0f;

    //This is basically to avoid shadow acne, i.e. self shadowing. But on the cost of Peter-shitty-Pan! Find your balance.
    float const_bias = 0.0002f;
    float multiplier = 0.002f;
    float bias = max(multiplier*(1.0f - dot(norm, light_dir_norm)), const_bias);
    if (current_frag_depth - bias > nearest_frag_depth)
        return 1.0f;
    return 0.0f;
}

void main()
{
    //Ambient color component.
    float ambient = 0.15f;

    //Diffuse color component.
    vec3 norm = normalize(normal);
    vec3 light_dir_norm = normalize(light_dir);
    float diffuse = max(dot(norm, light_dir_norm), 0.0f);

    //Shadow color component.
    float shadow = get_shadow(frag_pos_light, norm, light_dir_norm);

    frag_col = vec4((ambient + (1.0f - shadow)*diffuse)*mesh_col*light_col, 1.0f);
}
