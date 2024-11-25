#version 450 core

in vec3 frag_pos;
in vec3 normal;

out vec4 frag_col; //Final color of the fragment after lighting calculations.



uniform vec3 mesh_col; //Mesh color.
uniform vec3 light_pos; //Position of the light in world coordinates.
uniform vec3 light_col; //Light color.
uniform vec3 cam_pos; //Position of the camera in world coordinates.

void main()
{    
    //Ambient color component.
    float ambient = 0.15f;
    
    //Diffuse color component.
    vec3 norm = normalize(normal);
    vec3 light_dir_norm = normalize(light_pos - frag_pos); //Light direction with respect to the fragment.
    float diffuse = max(dot(norm, light_dir_norm), 0.0f);
    
    //Specular color component (shininess).
    vec3 view_dir_norm = normalize(cam_pos - frag_pos); //Camera's direction with respect to the fragment.
    vec3 reflect_dir_norm = reflect(-light_dir_norm, norm); //"Ray's" reflection direction with respect to the fragment.
    float specular = 0.5f*pow(max(dot(view_dir_norm, reflect_dir_norm), 0.0f), 128);
    
    //Attenuation factor.
    float light_dist = length(light_pos - frag_pos); //Distance between point light source and fragment.
    float k1 = 1.0f, k2 = 0.09f, k3 = 0.032f; //constant (k1), linear (k2) and quadratic (k3) attenuation parameters
    float atten_factor = 1.0f/(k1 + k2*light_dist + k3*light_dist*light_dist);
    
    frag_col = vec4((ambient + diffuse + specular)*mesh_col*light_col*atten_factor, 1.0f);
}
