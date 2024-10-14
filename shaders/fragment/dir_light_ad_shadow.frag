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
float get_shadow(vec4 frag_pos_light)
{
    vec3 projected_coords = frag_pos_light.xyz/frag_pos_light.w;
    projected_coords = 0.5f*projected_coords + 0.5f;

    float nearest_frag_depth = texture(sample_shadow, projected_coords.xy).r;
    float current_frag_depth = projected_coords.z;
    float bias = 0.001f;
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
    float shadow = get_shadow(frag_pos_light);

    frag_col = vec4((ambient + (1.0f - shadow)*diffuse)*mesh_col*light_col, 1.0f);
}
