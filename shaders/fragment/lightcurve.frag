#version 330 core

in vec3 frag_pos;
in vec3 normal;

out vec4 final_col; //color of the fragment after lighting calculations
out float brightness; // output brightness value instead of final color

uniform vec3 light_dir;
uniform vec3 light_col;
uniform vec3 model_col;
uniform vec3 cam_pos;

void main()
{
    vec3 norm = normalize(normal);
    vec3 light_dir_norm = normalize(light_dir);
    float diffuse = max(dot(norm, light_dir_norm), 0.0f);
    
    vec3 view_dir = normalize(cam_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir_norm,norm);
    float specular = 0.5f * pow(max(dot(view_dir, reflect_dir), 0.0f), 128);

    // Calculate brightness and output it
    brightness = length((diffuse + specular) * model_col * light_col);

    vec3 frag_col = (diffuse + specular)*model_col*light_col;
    final_col = vec4(frag_col, 1.0f);
}