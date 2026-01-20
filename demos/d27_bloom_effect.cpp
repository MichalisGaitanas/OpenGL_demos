#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include<cstdio>

#include"../include/shader.h"
#include"../include/mesh.h"

int win_width = 1920, win_height = 1080;

unsigned int scene_fbo = 0, ping_fbo = 0, pong_fbo = 0;
unsigned int ping_tex = 0, pong_tex = 0;
unsigned int scene_col = 0, scene_depth_rbo = 0; 

unsigned int create_rgba16f_tex(int w, int h)
{
    unsigned int tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

void setup_buffers(int w, int h)
{
    if (scene_fbo) {glDeleteFramebuffers(1, &scene_fbo); glDeleteTextures(1, &scene_col); glDeleteRenderbuffers(1, &scene_depth_rbo);}
    if (ping_fbo)  {glDeleteFramebuffers(1, &ping_fbo);  glDeleteTextures(1, &ping_tex);}
    if (pong_fbo)  {glDeleteFramebuffers(1, &pong_fbo);  glDeleteTextures(1, &pong_tex);}

    //Scene FBO (HDR color + depth) :
    glGenFramebuffers(1, &scene_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, scene_fbo);
    scene_col = create_rgba16f_tex(w,h);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, scene_col, 0);
    glGenRenderbuffers(1, &scene_depth_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, scene_depth_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, scene_depth_rbo);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fprintf(stderr, "Scene depth fbo is not completed!\n");
    
    //Ping :
    glGenFramebuffers(1, &ping_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, ping_fbo);
    ping_tex = create_rgba16f_tex(w/4,h/4);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ping_tex, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fprintf(stderr, "Ping fbo is not completed!\n");

    //Pong :
    glGenFramebuffers(1, &pong_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, pong_fbo);
    pong_tex = create_rgba16f_tex(w/4,h/4);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pong_tex, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fprintf(stderr, "Pong fbo is not completed!\n");

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
    setup_buffers(w,h);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 0); //Mind this. <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    GLFWwindow *window = glfwCreateWindow(win_width, win_height, "Bloom effect", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    //glfwGetWindowSize(window, &win_width, &win_height);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }

    meshvf asteroid("../obj/vf/uv_sphere_rad1_40x40.obj");
    quadtex screen_quad;

    shader sh_monochromatic("../shaders/vertex/trans_mvp.vert","../shaders/fragment/bloom_monochromatic.frag");
    shader sh_bright("../shaders/vertex/trans_nothing_texture.vert","../shaders/fragment/bloom_brightpass.frag");
    shader sh_blur("../shaders/vertex/trans_nothing_texture.vert","../shaders/fragment/bloom_blur.frag");
    shader sh_composite("../shaders/vertex/trans_nothing_texture.vert","../shaders/fragment/bloom_composite.frag");

    //Uniforms :
    glm::vec3 mesh_col = glm::vec3(1.0f,0.3f,0.0f);
    float bloom_intensity = 4.0f, bloom_strength = 0.2f, bloom_threshold = 1.0f, bloom_knee = 0.5f, bloom_radius = 1.0f;
    int blur_iterations = 6;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = NULL;
    io.Fonts->AddFontFromFileTTF("../fonts/Arial.ttf", 15.0f);
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");
    ImGuiStyle &imstyle = ImGui::GetStyle();
    imstyle.WindowMinSize = ImVec2(200.0f,200.0f);
    imstyle.FrameRounding = 5.0f;
    imstyle.WindowRounding = 5.0f;

    setup_buffers(win_width, win_height);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f,0.0f,0.0f,1.0f);

    while (!glfwWindowShouldClose(window))
    {
        glm::mat4 projection = glm::infinitePerspective(glm::radians(60.0f), (float)win_width/win_height, 0.05f);
        static float cam_dist = 5.0f, cam_lon = 270.0f, cam_lat = 90.0f;
        glm::vec3 cam_pos = cam_dist*glm::vec3(cos(glm::radians(cam_lon))*sin(glm::radians(cam_lat)),
                                               sin(glm::radians(cam_lon))*sin(glm::radians(cam_lat)),
                                               cos(glm::radians(cam_lat)));
        //cam_up vector is equal to the minus unit latitude basis vector (expressed as a function of the cartesian unit vectors). cam_up = -hat(θ(hat(x),hat(y),hat(z))).
        glm::vec3 cam_up = -glm::vec3(cos(glm::radians(cam_lat))*cos(glm::radians(cam_lon)),
                                      cos(glm::radians(cam_lat))*sin(glm::radians(cam_lon)),
                                     -sin(glm::radians(cam_lat)));
        glm::mat4 view = glm::lookAt(cam_pos, glm::vec3(0.0f), cam_up);
        //glm::mat4 model = glm::rotate(glm::mat4(1.0f), 1.0f*(float)glfwGetTime(), glm::vec3(0.0f,0.0f,1.0f));
        glm::mat4 model = glm::mat4(1.0f);

        //1) Scene -> HDR FBO
        glBindFramebuffer(GL_FRAMEBUFFER, scene_fbo);
        glViewport(0,0, win_width, win_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        sh_monochromatic.use();
        sh_monochromatic.set_mat4_uniform("projection", projection);
        sh_monochromatic.set_mat4_uniform("view", view);
        sh_monochromatic.set_mat4_uniform("model", model);
        sh_monochromatic.set_vec3_uniform("mesh_col", mesh_col);
        sh_monochromatic.set_float_uniform("bloom_intensity", bloom_intensity);
        asteroid.draw_triangles();

        //2) Bright-pass (scene -> ping)
        glBindFramebuffer(GL_FRAMEBUFFER, ping_fbo);
        glViewport(0,0, win_width/4, win_height/4);
        glClear(GL_COLOR_BUFFER_BIT);
        sh_bright.use();
        sh_bright.set_int_uniform("scene", 0);
        sh_bright.set_float_uniform("bloom_threshold", bloom_threshold);
        sh_bright.set_float_uniform("bloom_knee", bloom_knee);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, scene_col);
        screen_quad.draw_triangles(scene_col);

        //3) Blur (ping <-> pong)
        sh_blur.use();
        sh_blur.set_int_uniform("image",0);
        bool horizontal = true;
        for (int i = 0; i < blur_iterations; ++i)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, horizontal ? pong_fbo : ping_fbo);
            glViewport(0,0, win_width/4, win_height/4);
            glClear(GL_COLOR_BUFFER_BIT);
            sh_blur.set_bool_uniform("horizontal", horizontal);
            sh_blur.set_float_uniform("bloom_radius", bloom_radius);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, horizontal ? ping_tex : pong_tex);
            screen_quad.draw_triangles(horizontal ? ping_tex : pong_tex);
            horizontal = !horizontal;
        }

        unsigned int bloom_tex = (blur_iterations%2 == 0) ? ping_tex : pong_tex;

        //4) Composite -> default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0,0, win_width,win_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        sh_composite.use();
        sh_composite.set_int_uniform("scene", 0);
        sh_composite.set_int_uniform("bloom", 1);
        sh_composite.set_float_uniform("bloom_strength", bloom_strength);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, scene_col);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, bloom_tex);
        screen_quad.draw_triangles(scene_col); //Quad draws. Shader samples both units.

        //Render GUI :   

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(300.0f, 600.0f), ImGuiCond_FirstUseEver);
        static bool popen = true;
        ImGui::Begin("Controls", &popen); //Imgui window with title and a close button.
        if (!popen)
            glfwSetWindowShouldClose(window, true);
        ImGui::BulletText("Camera's position");
        ImGui::SliderFloat("dist [km]##cam_dist", &cam_dist, 1.0f, 10.0f);
        ImGui::SliderFloat("lon [deg]##cam_lon", &cam_lon, 0.0f, 360.0f);
        ImGui::SliderFloat("lat [deg]##cam_lat", &cam_lat, 0.0f, 180.0f);
        ImGui::BulletText("Bloom controls");
        ImGui::SliderFloat("intens ##bloom_intensity", &bloom_intensity, 0.0f, 20.0f);
        ImGui::SliderFloat("thresh ##bloom_threshold", &bloom_threshold, 0.0f, 5.0f);
        ImGui::SliderFloat("knee ##bloom_knee", &bloom_knee, 0.0f, 2.0f); 
        ImGui::SliderFloat("strength ##bloom_strength", &bloom_strength, 0.0f, 2.0f);
        ImGui::SliderFloat("radius ##bloom_radius", &bloom_radius, 1.0f, 8.0f);
        ImGui::SliderInt("iter ##blur_iterations", &blur_iterations, 1, 12); 
        ImGui::Text("FPS : [%.0f] ",ImGui::GetIO().Framerate);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
       
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}
