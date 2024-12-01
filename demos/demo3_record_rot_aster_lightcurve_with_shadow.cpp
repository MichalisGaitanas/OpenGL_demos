#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<cstdio>

#include"../include/shader.h"
#include"../include/mesh.h"
#include"../include/lightcurve.h"

class file_inputs
{
public:
    int win_width, win_height;
    int shadow_tex_reso;
    float ang_vel_z;
    float fov;
    float dir_light_lon, dir_light_lat;
};

const float PI = glm::pi<float>();

unsigned int fbo_depth, tex_depth; //IDs to hold the depth fbo and the depth texture (shadow map).
unsigned int fbo_lightcurve, rbo_lightcurve, tex_lightcurve; //IDs to hold the fbo, renderbuffer, and texture of the lightcurve.

bool find_assignment_operator(FILE *fp)
{
    int c = fgetc(fp);
    while (c != EOF)
    {
        if (c == ':')
        {
            c = fgetc(fp);
            if (c == '=')
                return true;
        }
        else
            c = fgetc(fp); //Update c when it is not ':'.
    }
    return false;
}

/*

Use as : FILE *fp = fopen("input.txt", "r");
         if (!fp)
             fprintf(stderr, "Failed to open file.");

         file_inputs inputs;

         // Read values after finding the ':=' pattern
         if (find_assignment_pattern(fp)) fscanf(fp, "%lf", &inputs.G);
         if (find_assignment_pattern(fp)) fscanf(fp, "%d", &inputs.count);

         fclose(fp);

         // Print to verify the read values
         printf("G: %lf\n", inputs.G);
         printf("Count: %d\n", inputs.count);
         
*/

void setup_fbo_depth(int shadow_tex_reso_x, int shadow_tex_reso_y)
{
    glGenFramebuffers(1, &fbo_depth);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_depth);
    glGenTextures(1, &tex_depth);
    glBindTexture(GL_TEXTURE_2D, tex_depth);
    //Shadow mapping is highly sensitive to depth precision, hence the 32 bits.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, shadow_tex_reso_x, shadow_tex_reso_y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL); //NULL because no texture data is provided yet.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float border_col[] = {1.0f, 1.0f, 1.0f, 1.0f}; //Pure white that is, coz white color corresponds to maximum depth.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);  
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_col);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex_depth, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fprintf(stderr, "Shadow framebuffer is not completed!\n");
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void setup_fbo_lightcurve(int win_width, int win_height)
{
    glGenFramebuffers(1, &fbo_lightcurve);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_lightcurve);
    glGenTextures(1, &tex_lightcurve);
    glBindTexture(GL_TEXTURE_2D, tex_lightcurve);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, win_width, win_height, 0, GL_RED, GL_FLOAT, NULL); //Grayscale values only (red channel only that is).
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_lightcurve, 0);
    glGenRenderbuffers(1, &rbo_lightcurve);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_lightcurve);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, win_width, win_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_lightcurve);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fprintf(stderr, "Lightcurve framebuffer is not completed!\n");
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); //Hide the window.

    int win_width = 800, win_height = 800;
    GLFWwindow *window = glfwCreateWindow(win_width, win_height, "Off-screen rendering", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f,0.0f,0.0f,1.0f);

    int shadow_tex_reso_x = 2048, shadow_tex_reso_y = 2048; //Shadow image resolution. Prefer 2k. 
    setup_fbo_depth(shadow_tex_reso_x, shadow_tex_reso_y);
    setup_fbo_lightcurve(win_width, win_height);

    meshvfn asteroid("../obj/vfn/suzanne.obj");
    shader shad_depth("../shaders/vertex/trans_dir_light_mvp.vert","../shaders/fragment/nothing.frag");
    shader shad_dir_light_with_shadow("../shaders/vertex/trans_mvpn_shadow.vert","../shaders/fragment/dir_light_d_shadow_cheap.frag");

    float fc = 1.1f, fl = 1.2; //Scale factors : fc is for the ortho cube size and fl for the directional light dummy distance.
    float rmax = asteroid.get_farthest_vertex_distance(); //[km]
    float dir_light_dist = fl*rmax; //[km]
    float ang_vel_z = PI/40.0f; //[rad/sec]
    float fov = PI/4.0f; //[rad]

    glm::mat4 projection = glm::infinitePerspective(fov, (float)win_width/win_height, 0.05f);
    glm::mat4 dir_light_projection = glm::ortho(-fc*rmax,fc*rmax, -fc*rmax,fc*rmax, (fl-fc)*rmax, 2.0f*fc*rmax); //Precomputed.

    float dir_light_lon = 0.0f, dir_light_lat = PI/2.0f;
    glm::vec3 light_dir = dir_light_dist*glm::vec3(cos(dir_light_lon)*sin(dir_light_lat),
                                                   sin(dir_light_lon)*sin(dir_light_lat),
                                                   cos(dir_light_lat));
    float dir_light_up_x = 0.0f, dir_light_up_y = 0.0f, dir_light_up_z = 1.0f;
    if (glm::abs(glm::normalize(light_dir).z) > 0.999f)
    {
        dir_light_up_y = 1.0f;
        dir_light_up_z = 0.0f;
    }

    glm::mat4 dir_light_view = glm::lookAt(light_dir, glm::vec3(0.0f), glm::vec3(dir_light_up_x, dir_light_up_y, dir_light_up_z));
    glm::mat4 dir_light_pv = dir_light_projection*dir_light_view; //Directional light's projection*view (total) matrix.

    float cam_dist = 10.0f*rmax, cam_lon = 3.0f*PI/2.0f, cam_lat = PI/1.5f;
    glm::vec3 cam_pos = cam_dist*glm::vec3(cos(cam_lon)*sin(cam_lat),
                                           sin(cam_lon)*sin(cam_lat),
                                           cos(cam_lat));
    glm::vec3 cam_up = -glm::vec3(cos(cam_lat)*cos(cam_lon),
                                  cos(cam_lat)*sin(cam_lon),
                                 -sin(cam_lat));
    glm::mat4 view = glm::lookAt(cam_pos, glm::vec3(0.0f), cam_up);

    //Pre-pass to the shaders some variables to avoid doing it in the rendering loop.
    shad_depth.use();
    shad_depth.set_mat4_uniform("dir_light_pv", dir_light_pv);
    shad_dir_light_with_shadow.use();
    shad_dir_light_with_shadow.set_mat4_uniform("projection", projection);
    shad_dir_light_with_shadow.set_mat4_uniform("view", view);
    shad_dir_light_with_shadow.set_mat4_uniform("dir_light_pv", dir_light_pv);
    shad_dir_light_with_shadow.set_vec3_uniform("light_dir", light_dir);

    //Lightcure data.
    float t0 = 0.0f, tmax = 1000.0f, dt = 1.0f;
    size_t i = 0;
    size_t sz = static_cast<size_t>((tmax - t0) / dt) + 1;
    std::vector<float> time_vector(sz); //[sec]
    std::vector<float> brightness_vector(sz);
    
    for (float t = t0; t <= tmax; t += dt, ++i)
    {
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), ang_vel_z*t, glm::vec3(0.0f,0.0f,1.0f));

        //1) Render to the depth framebuffer (used later for shadowing).
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_depth);
        glViewport(0,0, shadow_tex_reso_x,shadow_tex_reso_y);
        glClear(GL_DEPTH_BUFFER_BIT); //Only depth values exist in this framebuffer.
        shad_depth.use();
        shad_depth.set_mat4_uniform("model", model);
        asteroid.draw_triangles();

        //2) Render to the lightcurve framebuffer.
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_lightcurve); //Now we have both depth and color values (unlike to the fbo_depth).
        glViewport(0,0, win_width,win_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shad_dir_light_with_shadow.use();
        shad_dir_light_with_shadow.set_mat4_uniform("model", model);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_depth);
        shad_dir_light_with_shadow.set_int_uniform("sample_shadow", 0);
        asteroid.draw_triangles(); 
        glBindTexture(GL_TEXTURE_2D, 0);

        time_vector[i] = t; //[sec]
        brightness_vector[i] = get_brightness_gpu(tex_lightcurve, win_width, win_height);

        //Note : glfwSwapBuffers() is not needed because we're doing off-screen rendering. There's no need to have smooth frame transistions or sth...
        //Also glfwPollEvents() is not needed as well coz there's no any user input or window events. This is not an interactive simulation...
    }

    FILE *fp = fopen("lightcurve.txt","w");
    for (size_t i = 0; i < sz; ++i)
        fprintf(fp,"%f  %f\n",time_vector[i], brightness_vector[i]);
    fclose(fp);

    glfwTerminate();
    return 0;
}
