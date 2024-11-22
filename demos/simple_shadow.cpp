#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<cstdio>

#include"../include/shader.hpp"
#include"../include/mesh.hpp"
#include"../include/camera.hpp"

camera cam(glm::vec3(0.0f, -10.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), 90.0f); //Set the camera.

float time_tick; //Elapsed time per frame update.

float xpos_previous, ypos_previous;
bool first_time_entered_the_window = true;
bool cursor_visible = false;

int win_width = 1200, win_height = 900;

unsigned int fbo_depth, tex_depth; //IDs to hold the fbo and the depth texture (shadow map).

//Resolution of the shadow map texture. The higher, the better the final render of the shadow, but also more memory consumption and poorer performance.
//Think of it like a classical image creation. 1k, 2k, 4k, etc... The more pixels in the image, the higher its detail. The shadow map-tex (as we will see later),
//is literally an image creation and each pixel of this image holds some 'special' info (not rgb colors, but depth), that we use. For real time lighting and shadow,
//this image is recreated at each frame.
const int shadow_tex_reso_x = 8192, shadow_tex_reso_y = 8192; //4k image resolution.

void setup_fbo_depth()
{
    glGenFramebuffers(1, &fbo_depth); //Create fbo and assign ID.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_depth); //This means that all subsequent fb operations affect the fbo_depth.
    glGenTextures(1, &tex_depth); //Create tex and assign ID.
    glBindTexture(GL_TEXTURE_2D, tex_depth); //This means that all subsequent tex operations affect the tex_depth.
    //Actually create the depth texture with the specified resolution. Stored as floats and initialized as NULL because no data is provided yet.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, shadow_tex_reso_x, shadow_tex_reso_y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //The following ensures that when texture coordinates go outside [0,1] range, the border_col is used as depth-color.
    float border_col[] = {1.0f, 1.0f, 1.0f, 1.0f}; //Pure white that is, coz white color corresponds to maximum depth (remember ex21_depth_buffer.cpp).
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);  
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_col);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex_depth, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        printf("Hidden framebuffer not complete!\n");

    //Since shadow mapping only requires depth information and needs no colors, the following commnads make sure that
    //OpenGL avoids any (unnecessary) color buffer operations.
    glDrawBuffer(GL_NONE); //Don't draw to any color buffer, because we only care about the depth buffer.
    glReadBuffer(GL_NONE); //Don't attempt to read from any color buffer, because there isn't one in use.
    
    glBindTexture(GL_TEXTURE_2D, 0); //Unbind the tex_depth.
    glBindFramebuffer(GL_FRAMEBUFFER, 0); //Unbind the fbo_depth, and switch to the default fbo, i.e. the displayed in the monitor.
}

//For discrete keyboard events.
void key_callback(GLFWwindow *window, int key, int /*scancode*/, int action, int /*mods*/)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
        glfwSetWindowShouldClose(window, true);
}

//When the framebuffer resizes, do the following :
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
    //Setup glfw.
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    GLFWwindow *window = glfwCreateWindow(win_width, win_height, "Real time shadow", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowSizeLimits(window, 400, 400, GLFW_DONT_CARE, GLFW_DONT_CARE);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    glfwGetWindowSize(window, &win_width, &win_height);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }

    //Setup gui stuff. 
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = NULL; //Fucking .ini file!
    io.Fonts->AddFontFromFileTTF("../fonts/Arial.ttf", 15.0f);
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGuiStyle &imstyle = ImGui::GetStyle();
    imstyle.WindowMinSize = ImVec2(200.0f,200.0f);
    imstyle.FrameRounding = 5.0f;
    imstyle.WindowRounding = 5.0f;

    meshvfn asteroid("../obj/vfn/asteroids/gerasimenko256k.obj");
    float cube_margin_factor = 1.1f;
    float light_margin_factor = 1.2f;
    float half_ortho_cube_size = cube_margin_factor*asteroid.get_farthest_vertex_distance();

    //Shaders : 1 for the scene as perceived by the directional light and 1 for the scene as perceived by the camera. The first shader is gonna
    //be used to calculate a special info only (depth). The second shader is gonna use that info to compute all the fragment colors (ambient, diffuse, etc... AND shadows).
    shader shad_depth("../shaders/vertex/trans_dir_light_mvp.vert","../shaders/fragment/nothing.frag");
    shader shad_dir_light_with_shadow("../shaders/vertex/trans_mvpn_shadow.vert","../shaders/fragment/dir_light_d_shadow.frag");

    //This shader is only used to render the geometry model of the directional light in our scene.
    meshvf arrows("../obj/vf/dir_light_arrows.obj");
    shader shad_arrows("../shaders/vertex/trans_mvp.vert","../shaders/fragment/monochromatic.frag");

    setup_fbo_depth();

    //Constant mesh and light colors. We pass them to the shader from now to avoid doing it in the while loop...
    glm::vec3 mesh_col = glm::vec3(1.0f,1.0f,1.0f);
    glm::vec3 light_col = glm::vec3(1.0f,1.0f,1.0f);
    shad_dir_light_with_shadow.use();
    shad_dir_light_with_shadow.set_vec3_uniform("mesh_col", mesh_col);
    shad_dir_light_with_shadow.set_vec3_uniform("light_col", light_col);

    glm::mat4 dir_light_projection, dir_light_view, dir_light_pv; //Directional light's matrices.

    glm::mat4 projection, view, model; //Camera's matrices. The 'model' matrix is common.

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    float t0 = 0.0f, tnow;
    while (!glfwWindowShouldClose(window))
    {   

        /* Directional light definition in the code. */        

        //We want to simulate the shadow effects produced by a hypothetical infinitely far (directional) light. Since the light rays are considered to
        //be parallel, we define and map the shadows into a fixed size orthographic projection frustum. You can imagine it as cuboid (of fixed size),
        //geometrically defined with respect to the light's view space. As the light's direction changes over time (due to lookAt() using the light_dir),
        //the whole frustum rotates along with the light, always 'looking at' any target you set in lookAt() (2nd argument).
        static float dir_light_lon = 80.0f, dir_light_lat = 50.0f;
        glm::vec3 light_dir = light_margin_factor*asteroid.get_farthest_vertex_distance()*glm::vec3(cos(glm::radians(dir_light_lon))*sin(glm::radians(dir_light_lat)),
                                                             sin(glm::radians(dir_light_lon))*sin(glm::radians(dir_light_lat)),
                                                             cos(glm::radians(dir_light_lat)));
        glm::vec3 norm_light_dir = glm::normalize(light_dir);
        float dir_light_up_x = 0.0f, dir_light_up_y = 0.0f, dir_light_up_z = 1.0f;
        if (glm::abs(norm_light_dir.z) > 0.999f)
        {
            dir_light_up_y = 1.0f;
            dir_light_up_z = 0.0f;
        }

        dir_light_projection = glm::ortho(-half_ortho_cube_size,half_ortho_cube_size, -half_ortho_cube_size,half_ortho_cube_size, 0.01f, 2.0f*half_ortho_cube_size);
        dir_light_view = glm::lookAt(light_dir, glm::vec3(0.0f), glm::vec3(dir_light_up_x, dir_light_up_y, dir_light_up_z));
        dir_light_pv = dir_light_projection*dir_light_view; //Directional light's projection*view (total) matrix.

        //Camera's updated parameters.
        projection = glm::perspective(glm::radians(cam.fov), (float)win_width/win_height, 0.05f,500.0f);
        view = cam.view();

        //Bind the fbo_depth to render the shadow map.
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_depth);
        glViewport(0,0, shadow_tex_reso_x,shadow_tex_reso_y);
        glClear(GL_DEPTH_BUFFER_BIT); //Clear only depth, coz we write only depth in this buffer. There's no color attachment.
        shad_depth.use();
        shad_depth.set_mat4_uniform("dir_light_pv", dir_light_pv);
        //Now transform the models and render to the fbo_depth.
        model = glm::rotate(glm::mat4(1.0f), glm::radians(10.0f*(float)glfwGetTime()), glm::vec3(0.0f,0.0f,1.0f));
        shad_depth.set_mat4_uniform("model", model);
        asteroid.draw_triangles();

        //Bind the default fbo to render the scene to the window.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0,0, win_width, win_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Now we have both depth and color (unlike to the fbo_depth).
        shad_dir_light_with_shadow.use();
        shad_dir_light_with_shadow.set_mat4_uniform("projection", projection);
        shad_dir_light_with_shadow.set_mat4_uniform("view", view);
        shad_dir_light_with_shadow.set_vec3_uniform("light_dir", light_dir);
        shad_dir_light_with_shadow.set_mat4_uniform("dir_light_pv", dir_light_pv);
        glActiveTexture(GL_TEXTURE0); //Activate texture unit 0.
        glBindTexture(GL_TEXTURE_2D, tex_depth); //Bind tex_depth to texture unit 0.
        shad_dir_light_with_shadow.set_int_uniform("sample_shadow", 0); //Set sampler to use texture unit 0. This is handled automatically by OpenGL in case only 1 texture unit is used.
        //Now transform the models and render to the monitor.
        model = glm::rotate(glm::mat4(1.0f), glm::radians(10.0f*(float)glfwGetTime()), glm::vec3(0.0f,0.0f,1.0f));
        shad_dir_light_with_shadow.set_mat4_uniform("model", model);
        asteroid.draw_triangles();
        glBindTexture(GL_TEXTURE_2D, 0); //Unbind the tex_depth.

        
        

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
