/*

Here we generate the (synthetic lightcurve) of a rotating body.
We assume :
1) Custom 3D shape file for the body.
2) Rotation of the body around a fixed axis with constant angular velocity.
3) Diffuse lighting only with no self shadow.
4) Static (virtual) camera.

*/

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

const double pi = 3.141592653589793238462;

int win_width, win_height;
float aspect_ratio;
GLuint framebuffer, rbo, rendered_texture;

//calculate average brightness from the texture
float calculate_brightness(GLuint texture_id, int width, int height)
{
    glBindTexture(GL_TEXTURE_2D, texture_id);
    std::vector<GLfloat> pixels(width*height*3); //3 components for now (RGB)
    
    //read the pixels from the texture
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, pixels.data());
    
    //Sum up the intensity values (stored in the red channel)
    float total_brightness = 0.0f;
    for (int i = 0; i < width*height; i++)
    {
        total_brightness += pixels[i];
    }
    return total_brightness/(width*height); //average brightness
}

void setup_framebuffer(int width, int height)
{
    //if the framebuffer was already created, delete it first
    if (framebuffer)
    {
        glDeleteFramebuffers(1, &framebuffer);
        glDeleteTextures(1, &rendered_texture);
        glDeleteRenderbuffers(1, &rbo);
    }

    //create a framebuffer object (FBO)
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    //create a texture to render to
    glGenTextures(1, &rendered_texture);
    glBindTexture(GL_TEXTURE_2D, rendered_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, NULL);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //attach the texture to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rendered_texture, 0);
    
    //create a renderbuffer for depth and stencil
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        printf("Framebuffer not complete!\n");

    glBindFramebuffer(GL_FRAMEBUFFER, 0); //unbind the framebuffer to render to the default one
}

void raw_hardware_input(GLFWwindow *win)
{
    if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(win, true);
}

void framebuffer_size_callback(GLFWwindow *win, int w, int h)
{
    glViewport(0,0,w,h);
    win_width = w;
    win_height = h;
    aspect_ratio = (float)w/h;
    //re-setup the framebuffer (re-create the texture and renderbuffer with new size)
    setup_framebuffer(w,h);
}

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    
    GLFWwindow *win = glfwCreateWindow(800, 600, "Synthetic lightcurve", NULL, NULL);
    if (win == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    
    glfwMakeContextCurrent(win);
    
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        glfwTerminate();
        return 0;
    }

    ///////////////////////////////////////////////////////////////////////////////////

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext(); //strictly AFTER Imgui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = NULL;
    io.Fonts->AddFontFromFileTTF("../fonts/arial.ttf", 15.0f);
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(win, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    ///////////////////////////////////////////////////////////////////////////////////

    //change the default minimum size of an imgui window
    ImGuiStyle &imstyle = ImGui::GetStyle();
    imstyle.WindowMinSize = ImVec2(300.0f,300.0f);

    glfwSetWindowSizeLimits(win, 400, 400, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);
    
    glfwGetWindowSize(win, &win_width, &win_height);
    aspect_ratio = (float)win_width/win_height;

    meshvfn body("../obj/vfn/gerasimenko256k.obj");
    //meshvfn body("../obj/vfn/uv_sphere_rad1_40x30.obj"); 

    shader shad("../shaders/vertex/trans_mvpn.vert","../shaders/fragment/lightcurve.frag");
    shad.use();

    glm::vec3 light_dir = glm::vec3(-1.0f,0.0f,0.0f); //light direction in world coordinates
    glm::vec3 light_col = glm::vec3(1.0f,1.0f,1.0f); //lighting calculation color
    
    glm::vec3 cam_pos = glm::vec3(0.0f,-7.0f,2.0f); //camera position in world coordinates
    glm::vec3 cam_aim = glm::vec3(0.0f,0.0f,0.0f); //camera aim direction
    glm::vec3 cam_up = glm::vec3(0.0f,0.0f,1.0f); //camera up direction

    //projection, view and model matrix
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)win_width/win_height, 0.01f,1000.0f);
    glm::mat4 view = glm::lookAt(cam_pos, cam_aim, cam_up);
    glm::mat4 model = glm::mat4(1.0f);

    shad.set_mat4_uniform("projection", projection);
    shad.set_mat4_uniform("view", view);
    shad.set_mat4_uniform("model", model);
    shad.set_vec3_uniform("cam_pos", cam_pos);
    shad.set_vec3_uniform("light_dir", light_dir);
    shad.set_vec3_uniform("light_col", light_col);

    setup_framebuffer(win_width, win_height);
    std::vector<float> current_time;
    std::vector<float> lightcurve;
    bool show_realtime_lightcurve = false;

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    while (!glfwWindowShouldClose(win))
    {
        double tt = glfwGetTime();

        raw_hardware_input(win);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        model = glm::mat4(1.0f);
        model = glm::rotate(model, (float)(10*tt*pi/180.0f), glm::vec3(0.0f,0.0f,1.0f));
        shad.set_mat4_uniform("model", model);
        body.draw_triangles();

        //append new data points
        current_time.push_back(tt);
        float brightness = calculate_brightness(rendered_texture, win_width, win_height);
        lightcurve.push_back(brightness);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        body.draw_triangles();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Gerasimenko Lightcurve");

        ImGui::Checkbox("Click to plot real-time lightcurve", &show_realtime_lightcurve);
        ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); //red color for the plot curves
        
        if (show_realtime_lightcurve)
        {
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y), ImGuiCond_FirstUseEver);
            ImGui::Begin("Real-time lightcurve", &show_realtime_lightcurve);
            ImVec2 plot_win_size = ImVec2(ImGui::GetWindowSize().x - 20.0f, ImGui::GetWindowSize().y - 40.0f);
            if (ImPlot::BeginPlot("brightness(t)", plot_win_size))
            {
                ImPlot::SetupAxes("Time (t) [sec]", "brightness(t) [ ]");
                //ImPlot::SetupAxisLimits(ImAxis_X1, start_time, current_time, ImGuiCond_Always); // Scroll with time
                ImPlot::PlotLine("", current_time.data(), lightcurve.data(), current_time.size());
                ImPlot::EndPlot();
            }
            ImGui::End();
        }

        ImPlot::PopStyleColor();
        
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
