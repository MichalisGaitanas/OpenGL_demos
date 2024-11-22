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
#include<cmath>
#include<vector>

#include"../include/shader.hpp"
#include"../include/mesh.hpp"

int win_width = 1000, win_height = 1000; //Initial glfw window size.
unsigned int fbo, rbo, tex; //Framebuffer object, renderbuffer object and texture ID.

//Calculate brightness (lightcurve) from the rendered scene in the hidden framebuffer (fbo).
float get_brightness(unsigned int tex, int width_pix, int height_pix)
{
    glBindTexture(GL_TEXTURE_2D, tex);
    std::vector<float> pixels(width_pix*height_pix);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, pixels.data()); //Read the pixels from the texture (only the red channel, i.e. grayscale color).
    float brightness = 0.0f;
    for (int i = 0; i < width_pix*height_pix; ++i)
        brightness += pixels[i];
    return brightness/(width_pix*height_pix); //Normalize the brightness.
}

//Create a new auxiliary hidden framebuffer (fbo), that we will use to perform the lightcurve calculation.
void setup_fbo(int width_pix, int height_pix)
{
    //If memory resources are already allocated, delete them first.
    if (fbo)
    {
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &tex);
        glDeleteRenderbuffers(1, &rbo);
    }

    //Create a framebuffer object (fbo). This is basically similar as the process of creating vbo, vao, ebo, etc...
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    //Create a texture (tex) to render to.
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width_pix, height_pix, 0, GL_RED, GL_FLOAT, NULL); //Grayscale values only (red channel only that is).
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //Attach the texture to the hidden framebuffer.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    
    //Create a renderbuffer for depth and stencil.
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width_pix, height_pix);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        printf("Warning : Framebuffer (fbo) is not completed.\n");

    //The hidden framebuffer is now created. We refer to it from now by binding/unbinding.

    //Unbind the hidden framebuffer to render to the default one (0).
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//When a keyboard key is pressed, do the following :
void key_callback(GLFWwindow *win, int key, int, int action, int)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
        glfwSetWindowShouldClose(win, true); //Kill the game loop when 'esc' is released.
}

//When the framebuffer is resized, do the following :
void framebuffer_size_callback(GLFWwindow *, int w, int h)
{
    if (w < 1) w = 1;
    if (h < 1) h = 1;
    win_width = w;
    win_height = h;
    glViewport(0,0,w,h);
    setup_fbo(w,h); //Re-setup the hidden framebuffer. This basically guarantees the re-creation of the texture and renderbuffer with new size.
}

int main()
{
    //Setup glfw stuff.
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    GLFWwindow *win = glfwCreateWindow(win_width, win_height, "Lightcurve generation", NULL, NULL);
    if (win == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(win);

    glfwSetWindowSizeLimits(win, 400, 400, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);
    glfwSetKeyCallback(win, key_callback);
    glfwGetWindowSize(win, &win_width, &win_height);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        glfwTerminate();
        return 0;
    }

    //Setup gui stuff.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext(); //strictly AFTER Imgui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = NULL; //Fucking .ini file!
    io.Fonts->AddFontFromFileTTF("../fonts/Arial.ttf", 15.0f);
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(win, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    meshvfn aster("../obj/vfn/asteroids/gerasimenko256k.obj");

    shader shad("../shaders/vertex/trans_mvpn.vert","../shaders/fragment/dir_light_d.frag");
    shad.use();

    float ang_vel_z = 0.01f; //[rad/sec]

    //Directional light parameters.
    float angle = 180.0f; //Exposed in the gui.
    glm::vec3 light_dir = glm::vec3(cos(glm::radians(angle)), sin(glm::radians(angle)), 0.0f); //Light direction in 'world' coordinates.
    glm::vec3 light_col = glm::vec3(1.0f,1.0f,1.0f); //Light color (white).
    glm::vec3 mesh_col = glm::vec3(1.0f,1.0f,1.0f); //Asteroid color (white).

    //Camera parameters.
    glm::vec3 cam_pos = glm::vec3(0.0f,-10.0f,0.0f); //Camera's position in 'world' coordinates. Exposed in the gui.
    glm::vec3 cam_aim = glm::vec3(0.0f,0.0f,0.0f); //Camera aims at the origin of our 'world' coordsys.
    glm::vec3 cam_up = glm::vec3(0.0f,0.0f,1.0f); //Camera's local up direction vector (+z).
    float fov = 45.0f; //Camera's field of view. Exposed in the gui.

    //Actual lightcure data.
    std::vector<float> time_vector;
    std::vector<float> brightness_vector;

    //Transformation matrices that take us from the 3D 'world' coordsys to the 2D monitor pixeled coordsys.
    glm::mat4 projection, view, model;

    //Now inform the shader about the uniforms that are NOT going to change in the rendering loop.
    shad.set_vec3_uniform("light_col", light_col);
    shad.set_vec3_uniform("mesh_col", mesh_col);

    //Create the (clean) hidden framebuffer.
    setup_fbo(win_width, win_height);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE); //Enable face culling (better performance).
    glClearColor(0.0f,0.0f,0.0f,1.0f);

    float t = 0.0f, dt = 1.0f, tmax = 200.0f; //[sec]
    while (!glfwWindowShouldClose(win) && t <= tmax)
    {
        projection = glm::infinitePerspective(glm::radians(fov), (float)win_width/win_height, 1.0f); //1.0f = znear
        shad.set_mat4_uniform("projection", projection);

        view = glm::lookAt(cam_pos, cam_aim, cam_up);
        shad.set_mat4_uniform("view", view);

        //Bind the hidden framebuffer and render the scene there.
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        model = glm::rotate(glm::mat4(1.0f), ang_vel_z*t, glm::vec3(0.0f,0.0f,1.0f));
        shad.set_mat4_uniform("model", model);
        aster.draw_triangles();

        //The scene is now rendered in the hidden framebuffer. It will not be displayed on the monitor.
        //With that scene rendered, let's calculate the lightcurve data (time and brightness) :
        time_vector.push_back(t);
        brightness_vector.push_back(get_brightness(tex, win_width, win_height));
        t += dt;
        //The calculation of the lightvurve is over for this frame. The 'backend' hidden framebuffer holds the important info.
        //You may print the results in a file.

        //If we want to display the scene in the monitor as well (the default framebuffer), then :
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        aster.draw_triangles();

        //Render gui stuff.

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
 
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Menu", nullptr);

        ImGui::Text("Light direction");
        ImGui::SliderFloat("[deg]##angle", &angle, 0.0f, 360.0f);
        ImGui::Separator();
        light_dir.x = cos(glm::radians(angle));
        light_dir.y = sin(glm::radians(angle));
        shad.set_vec3_uniform("light_dir", light_dir);

        ImGui::Text("Camera y-coord");
        ImGui::SliderFloat("[km]", &cam_pos.y, -250.0f,-1.0f);
        ImGui::Separator();
        shad.set_vec3_uniform("cam_pos", cam_pos);
        view = glm::lookAt(cam_pos, cam_aim, cam_up);
        shad.set_mat4_uniform("view", view);

        ImGui::Text("Camera f.o.v.");
        ImGui::SliderFloat("[deg]##fov", &fov, 1.0f, 45.0f);
        ImGui::Separator();
        projection = glm::perspective(glm::radians(fov), (float)win_width/win_height, 0.01f, 3000.0f);
        shad.set_mat4_uniform("projection", projection);

        ImGui::Dummy(ImVec2(0.0f,15.0f));

        ImGui::Text("FPS [ %.0f ] ", ImGui::GetIO().Framerate);

        ImGui::End(); 

        ImGui::SetNextWindowPos(ImVec2(0.0f, 5.5f*ImGui::GetIO().DisplaySize.y/7.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(5.0f*ImGui::GetIO().DisplaySize.x/7.0f, 1.5f*ImGui::GetIO().DisplaySize.y/7.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Real-time lightcurve", nullptr);
        ImVec2 plot_win_size = ImVec2(ImGui::GetWindowSize().x - 20.0f, ImGui::GetWindowSize().y - 40.0f);
        if (ImPlot::BeginPlot("Lightcurve", plot_win_size))
        {
            ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.75f, 0.0f, 0.0f, 1.0f)); //Red color for the lightcurve.
            ImPlot::SetupAxes("Time [sec]", "Brightness [norm]");
            ImPlot::SetupAxisLimits(ImAxis_X1, t-70.0f, t, ImGuiCond_Always); //Automatically scroll with time along the t-axis.
            ImPlot::PlotLine("", time_vector.data(), brightness_vector.data(), time_vector.size());
            ImPlot::PopStyleColor();
            ImPlot::EndPlot();
        }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    FILE *fp_lightcurve = fopen("demo4_lightcurve.txt","w");
    for (size_t i = 0; i < time_vector.size(); ++i)
        fprintf(fp_lightcurve,"%.6f  %.6f\n",time_vector[i], brightness_vector[i]);
    fclose(fp_lightcurve);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext(); //Strictly BEFORE Imgui::DestroyContext();
    ImGui::DestroyContext();

    glfwTerminate();

    return 0;
}
