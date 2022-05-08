#version 330 core

in vec3 frag_pos;
in vec3 normal;

out vec4 final_col; //color of the fragment after lighting calculations

uniform vec3 light_pos; //position of the light in world coordinates
uniform vec3 light_col; //light color
uniform vec3 model_col; //model color
uniform vec3 cam_front; //view front vector
uniform float cos_cuttoff; //cosine of the cone angle (separates the flashlight calculations with the rest of the world's lighting calculations)

void main()
{
    //ambient color component
    float ambient = 0.15f;
    
    vec3 light_dir = normalize(light_pos - frag_pos);
    float cos_frag = dot(-light_dir, normalize(cam_front));
    float diffuse; //diffuse color component
    if (cos_frag >= cos_cuttoff)
    {
        vec3 norm = normalize(normal);
        diffuse = max(dot(norm, light_dir), 0.0f);
    }
    else
    {
        diffuse = 0.0f;
    }
    
    //final fragment color
    vec3 frag_col = (ambient + diffuse)*model_col*light_col;
    final_col = vec4(frag_col, 1.0f);
}





