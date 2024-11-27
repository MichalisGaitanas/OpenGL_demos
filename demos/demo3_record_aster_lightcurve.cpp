#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<cstdio>

#include"../include/shader.h"
#include"../include/mesh.h"
#include"../include/lightcurve.h"

const float PI = glm::pi<float>();

int win_width = 1920, win_height = 1080;

const int shadow_tex_reso_x = 2048, shadow_tex_reso_y = 2048; //Shadow image resolution.

unsigned int fbo_depth, tex_depth; //IDs to hold the depth fbo and the depth texture (shadow map).
unsigned int fbo_lightcurve, rbo_lightcurve, tex_lightcurve; //IDs to hold the fbo, renderbuffer, and texture of the lightcurve.

void setup_fbo_depth()
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

void setup_fbo_lightcurve(int width_pix, int height_pix)
{
    if (fbo_lightcurve)
    {
        glDeleteFramebuffers(1, &fbo_lightcurve);
        glDeleteTextures(1, &tex_lightcurve);
        glDeleteRenderbuffers(1, &rbo_lightcurve);
    }
    glGenFramebuffers(1, &fbo_lightcurve);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_lightcurve);
    glGenTextures(1, &tex_lightcurve);
    glBindTexture(GL_TEXTURE_2D, tex_lightcurve);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width_pix, height_pix, 0, GL_RED, GL_FLOAT, NULL); //Grayscale values only (red channel only that is).
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, width_pix, height_pix, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_lightcurve, 0);
    glGenRenderbuffers(1, &rbo_lightcurve);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_lightcurve);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width_pix, height_pix);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_lightcurve);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fprintf(stderr, "Lightcurve framebuffer is not completed!\n");
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void key_callback(GLFWwindow *window, int key, int /*scancode*/, int action, int /*mods*/)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow */*win*/, int w, int h)
{
    if (w < 1) w = 1;
    if (h < 1) h = 1;
    win_width = w;
    win_height = h;
    glViewport(0,0,w,h);
    setup_fbo_lightcurve(w,h); //Re-setup the lightcurve framebuffer. This basically guarantees the re-creation of the texture and renderbuffer with new size.
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    GLFWwindow *window = glfwCreateWindow(win_width, win_height, "Asteroid rotational lightcurve (with shadow)", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwGetWindowSize(window, &win_width, &win_height);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }

    meshvfn asteroid("../obj/vfn/asteroids/kleopatra4k.obj");
    shader shad_depth("../shaders/vertex/trans_dir_light_mvp.vert","../shaders/fragment/nothing.frag");
    shader shad_dir_light_with_shadow("../shaders/vertex/trans_mvpn_shadow.vert","../shaders/fragment/dir_light_d_shadow_cheap.frag");

    setup_fbo_depth();
    setup_fbo_lightcurve(win_width, win_height);

    glm::vec3 mesh_col = glm::vec3(1.0f,1.0f,1.0f);
    glm::vec3 light_col = glm::vec3(1.0f,1.0f,1.0f);
    shad_dir_light_with_shadow.use();
    shad_dir_light_with_shadow.set_vec3_uniform("mesh_col", mesh_col);
    shad_dir_light_with_shadow.set_vec3_uniform("light_col", light_col);

    float fc = 1.1f, fl = 1.2; //Scale factors : fc is for the ortho cube size and fl for the directional light dummy distance.
    float rmax = asteroid.get_farthest_vertex_distance(); //[km]
    float dir_light_dist = fl*rmax; //[km]
    float ang_vel_z = PI/40.0f; //[rad/sec]
    float fov = PI/4.0f; //[rad]

    glm::mat4 dir_light_projection = glm::ortho(-fc*rmax,fc*rmax, -fc*rmax,fc*rmax, (fl-fc)*rmax, 2.0f*fc*rmax); //Precomputed.

    //Lightcure data.
    float t0 = 0.0f, tmax = 1000.0f, dt = 1.0f;
    size_t i = 0;
    size_t sz = static_cast<int>((tmax - t0)/dt) + 1;
    std::vector<float> time_vector(sz); //[sec]
    std::vector<float> brightness_vector(sz);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    for (float t = t0; t <= tmax; t += dt, ++i)
    {
        //Essential calculation needed for rendering :

        static float dir_light_lon = 0.0f, dir_light_lat = PI/4.0f;
        glm::vec3 light_dir = dir_light_dist*glm::vec3(cos(dir_light_lon)*sin(dir_light_lat),
                                                       sin(dir_light_lon)*sin(dir_light_lat),
                                                       cos(dir_light_lat));
        glm::vec3 norm_light_dir = glm::normalize(light_dir);
        float dir_light_up_x = 0.0f, dir_light_up_y = 0.0f, dir_light_up_z = 1.0f;
        if (glm::abs(norm_light_dir.z) > 0.999f)
        {
            dir_light_up_y = 1.0f;
            dir_light_up_z = 0.0f;
        }
        glm::mat4 dir_light_view = glm::lookAt(light_dir, glm::vec3(0.0f), glm::vec3(dir_light_up_x, dir_light_up_y, dir_light_up_z));
        glm::mat4 dir_light_pv = dir_light_projection*dir_light_view; //Directional light's projection*view (total) matrix.

        glm::mat4 projection = glm::infinitePerspective(fov, (float)win_width/win_height, 0.05f);
        static float cam_dist = 5.0f*rmax, cam_lon = 3.0f*PI/2.0f, cam_lat = PI/2.0f;
        glm::vec3 cam_pos = cam_dist*glm::vec3(cos(cam_lon)*sin(cam_lat),
                                               sin(cam_lon)*sin(cam_lat),
                                               cos(cam_lat));
        //cam_up vector is equal to the minus unit latitude basis vector (expressed as a function of the cartesian unit vectors). cam_up = -hat(θ(hat(x),hat(y),hat(z))).
        glm::vec3 cam_up = -glm::vec3(cos(cam_lat)*cos(cam_lon),
                                      cos(cam_lat)*sin(cam_lon),
                                     -sin(cam_lat));
        glm::mat4 view = glm::lookAt(cam_pos, glm::vec3(0.0f), cam_up);

        glm::mat4 model = glm::rotate(glm::mat4(1.0f), ang_vel_z*t, glm::vec3(0.0f,0.0f,1.0f));

        //Now we render :

        //1) Render to the depth framebuffer (used later for shadowing).
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_depth);
        glViewport(0,0, shadow_tex_reso_x,shadow_tex_reso_y);
        glClear(GL_DEPTH_BUFFER_BIT); //Only depth values exist in this framebuffer.
        shad_depth.use();
        shad_depth.set_mat4_uniform("dir_light_pv", dir_light_pv);
        shad_depth.set_mat4_uniform("model", model);
        asteroid.draw_triangles();

        //2) Render to the lightcurve framebuffer.
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_lightcurve); //Now we have both depth and color values (unlike to the fbo_depth).
        glViewport(0,0, win_width,win_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shad_dir_light_with_shadow.use();
        shad_dir_light_with_shadow.set_mat4_uniform("projection", projection);
        shad_dir_light_with_shadow.set_mat4_uniform("view", view);
        shad_dir_light_with_shadow.set_mat4_uniform("model", model);
        shad_dir_light_with_shadow.set_mat4_uniform("dir_light_pv", dir_light_pv);
        shad_dir_light_with_shadow.set_vec3_uniform("light_dir", light_dir);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_depth);
        shad_dir_light_with_shadow.set_int_uniform("sample_shadow", 0);
        asteroid.draw_triangles(); 
        glBindTexture(GL_TEXTURE_2D, 0);

        time_vector[i] = t; //[sec]
        brightness_vector[i] = get_brightness_gpu(tex_lightcurve, win_width, win_height);
       
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    FILE *fp = fopen("lightcurve.txt","w");
    for (size_t i = 0; i < sz; ++i)
        fprintf(fp,"%f  %f\n",time_vector[i], brightness_vector[i]);
    fclose(fp);

    glfwTerminate();
    return 0;
}
