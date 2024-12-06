#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"
#include"../imgui/implot.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include<cstdio>

#include"../include/shader.h"
#include"../include/mesh.h"

const float PI = glm::pi<float>();

int win_width = 1920, win_height = 1080;

const int shadow_tex_reso_x = 4096, shadow_tex_reso_y = 4096; //Shadow image resolution.

unsigned int fbo_depth, tex_depth; //IDs to hold the depth fbo and the depth texture (shadow map).

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
        fprintf(stderr, "Depth framebuffer is not completed!\n");
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
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
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
    GLFWwindow *window = glfwCreateWindow(win_width, win_height, "Asteroid rendering", NULL, NULL);
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

    meshvfn asteroid("../obj/vfn/asteroids/gerasimenko256k.obj");
    shader shad_depth("../shaders/vertex/trans_dir_light_mvp.vert","../shaders/fragment/nothing.frag");
    shader shad_dir_light_with_shadow("../shaders/vertex/trans_mvpn_shadow.vert","../shaders/fragment/dir_light_d_shadow.frag");

    setup_fbo_depth();

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

    glm::vec3 mesh_col = glm::vec3(1.0f,1.0f,1.0f);
    glm::vec3 light_col = glm::vec3(1.0f,1.0f,1.0f);
    shad_dir_light_with_shadow.use();
    shad_dir_light_with_shadow.set_vec3_uniform("mesh_col", mesh_col);
    shad_dir_light_with_shadow.set_vec3_uniform("light_col", light_col);

    float fc = 1.1f, fl = 1.2; //Scale factors : fc is for the ortho cube size and fl for the directional light dummy distance.
    float rmax = asteroid.get_farthest_vertex_distance(); //[km]
    float dir_light_dist = fl*rmax; //[km]
    float fov = 45.0f; //[deg]
    float t = 0.0f, dt = 1.0f; //[sec]

    glm::mat4 dir_light_projection = glm::ortho(-fc*rmax,fc*rmax, -fc*rmax,fc*rmax, (fl-fc)*rmax, 2.0f*fc*rmax); //Precomputed.

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f,0.0f,0.0f,1.0f);

    while (!glfwWindowShouldClose(window))
    {
        //Essential calculation needed for rendering :

        static float dir_light_lon = 0.0f, dir_light_lat = 90.0f;
        glm::vec3 light_dir = dir_light_dist*glm::vec3(cos(glm::radians(dir_light_lon))*sin(glm::radians(dir_light_lat)),
                                                       sin(glm::radians(dir_light_lon))*sin(glm::radians(dir_light_lat)),
                                                       cos(glm::radians(dir_light_lat)));
        float dir_light_up_x = 0.0f, dir_light_up_y = 0.0f, dir_light_up_z = 1.0f;
        if (glm::abs(glm::normalize(light_dir).z) > 0.999f)
        {
            dir_light_up_y = 1.0f;
            dir_light_up_z = 0.0f;
        }
        glm::mat4 dir_light_view = glm::lookAt(light_dir, glm::vec3(0.0f), glm::vec3(dir_light_up_x, dir_light_up_y, dir_light_up_z));
        glm::mat4 dir_light_pv = dir_light_projection*dir_light_view; //Directional light's projection*view (total) matrix.

        glm::mat4 projection = glm::infinitePerspective(glm::radians(fov), (float)win_width/win_height, 0.05f);
        static float cam_dist = 5.0f*rmax, cam_lon = 270.0f, cam_lat = 90.0f;
        glm::vec3 cam_pos = cam_dist*glm::vec3(cos(glm::radians(cam_lon))*sin(glm::radians(cam_lat)),
                                               sin(glm::radians(cam_lon))*sin(glm::radians(cam_lat)),
                                               cos(glm::radians(cam_lat)));
        //cam_up vector is equal to the minus unit latitude basis vector (expressed as a function of the cartesian unit vectors). cam_up = -hat(θ(hat(x),hat(y),hat(z))).
        glm::vec3 cam_up = -glm::vec3(cos(glm::radians(cam_lat))*cos(glm::radians(cam_lon)),
                                      cos(glm::radians(cam_lat))*sin(glm::radians(cam_lon)),
                                     -sin(glm::radians(cam_lat)));
        glm::mat4 view = glm::lookAt(cam_pos, glm::vec3(0.0f), cam_up);

        glm::mat4 model = glm::rotate(glm::mat4(1.0f), 0.1f*(float)glfwGetTime(), glm::vec3(0.0f,0.0f,1.0f));

        //Now we render :

        //1) Render to the depth framebuffer (used later for shadowing).
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_depth);
        glViewport(0,0, shadow_tex_reso_x,shadow_tex_reso_y);
        glDisable(GL_FRAMEBUFFER_SRGB);
        glClear(GL_DEPTH_BUFFER_BIT); //Only depth values exist in this framebuffer.
        shad_depth.use();
        shad_depth.set_mat4_uniform("dir_light_pv", dir_light_pv);
        shad_depth.set_mat4_uniform("model", model);
        asteroid.draw_triangles();

        //2) Render to the default framebuffer (monitor).
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0,0, win_width,win_height);
        static bool apply_gamma_correction = false;
        if (apply_gamma_correction)
            glEnable(GL_FRAMEBUFFER_SRGB);
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

        t += dt; //[sec]

        //Render GUI :   

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(300.0f, 600.0f), ImGuiCond_FirstUseEver);
        static bool popen = true;
        ImGui::Begin("Controls", &popen); //Imgui window with title and a close button.
        if (!popen)
            glfwSetWindowShouldClose(window, true);
        ImGui::BulletText("Light's direction");
        ImGui::SliderFloat("lon [deg]##dir_light_lon", &dir_light_lon, 0.0f, 360.0f);
        ImGui::SliderFloat("lat [deg]##dir_light_lat", &dir_light_lat, 0.0f, 180.0f);
        ImGui::BulletText("Camera's position");
        ImGui::SliderFloat("dist [km]##cam_dist", &cam_dist, 2.0f*rmax, 50.0f*rmax); //The camera distance ranges from 2 to 50 times the distance of the farthest vertex of the mesh.
        ImGui::SliderFloat("lon [deg]##cam_lon", &cam_lon, 0.0f, 360.0f);
        ImGui::SliderFloat("lat [deg]##cam_lat", &cam_lat, 0.0f, 180.0f);
        ImGui::BulletText("Gamma correction");
        ImGui::Checkbox("Apply", &apply_gamma_correction);
        ImGui::BulletText("Performance");
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
