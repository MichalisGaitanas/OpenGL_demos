/*

Here we generate the (synthetic lightcurve) of a rotating body.
We add to lightcurve_stage1.cpp the following :
1) Menu to choose the 3D shape file for the body.
2) Movable camera.
3) The lighcurve responds to the camera at every frame

*/

//#include"../imgui/imgui.h"
//#include"../imgui/imgui_impl_glfw.h"
//#include"../imgui/imgui_impl_opengl3.h"
//#include"../imgui/implot.h"

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
#include"../include/camera.hpp"

const double pi = 3.141592653589793238462;

float t_init, t_now, delta_time;
float xpos_previous, ypos_previous;
bool first_time_entered_the_window = true;
float fov = 45.0f;

int win_width, win_height;
float aspect_ratio;

unsigned int framebuffer, rbo, rendered_texture;

camera cam;

//Calculate brightness (lightcurve) from the rendered scene of the (hidden) framebuffer.
float calculate_brightness(unsigned int texture_id, int width, int height)
{
    glBindTexture(GL_TEXTURE_2D, texture_id);
    std::vector<float> pixels(width*height);
    
    //Read the pixels from the texture (only the red channel, i.e. grayscale color).
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, pixels.data());
    
    //Sum up the intensity values stored in the red channel.
    float brightness = 0.0f;
    for (int i = 0; i < width*height; i++)
    {
        brightness += pixels[i];
    }
    return brightness/(width*height); //average brightness
}

//Create a new auxiliary framebuffer, that we will use to perform the lightcurve calculation.
void setup_framebuffer(int width, int height)
{
    //If the framebuffer was already created, delete it first.
    if (framebuffer)
    {
        glDeleteFramebuffers(1, &framebuffer);
        glDeleteTextures(1, &rendered_texture);
        glDeleteRenderbuffers(1, &rbo);
    }

    //Create a framebuffer object (fbo). This is basically similar as the process of creating vbo, vao, ebo, etc...
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    //Create a texture to render to.
    glGenTextures(1, &rendered_texture);
    glBindTexture(GL_TEXTURE_2D, rendered_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, NULL);    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //Attach the texture to the framebuffer.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rendered_texture, 0);
    
    //Create a renderbuffer for depth and stencil.
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        printf("Framebuffer not complete!\n");
    
    //The auxiliary framebuffer is now created. We refer to it from now via binding or unbinding.

    //Unbind the auxiliary framebuffer to render to the default one (0).
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//Translate the camera with a,s,d,w keys.
void raw_hardware_input(GLFWwindow *win)
{
    if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(win, true);

    if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS)
        cam.translate_front(delta_time);
    if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
        cam.translate_back(delta_time);
    if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
        cam.translate_right(delta_time);
    if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
        cam.translate_left(delta_time);
}

//Rotate the camera with the mouse.
void cursor_pos_callback(GLFWwindow *win, double xpos, double ypos)
{
    if (first_time_entered_the_window)
    {
        xpos_previous = xpos;
        ypos_previous = ypos;
        first_time_entered_the_window = false;
    }

    float xoffset = xpos - xpos_previous;
    float yoffset = ypos - ypos_previous;

    xpos_previous = xpos;
    ypos_previous = ypos;

    cam.rotate(xoffset, yoffset);
}

//Zoom in/out via the mouse wheel (basically we change the fov of the camera).
void scroll_callback(GLFWwindow *win, double xoffset, double yoffset)
{
    fov -= 2.0f*(float)yoffset;
    if (fov <= 1.0f)
        fov = 1.0f;
    else if (fov >= 45.0f)
        fov = 45.0f;
}

//Framebuffer callback. That is, when the window is resized, the following commands will execute.
void framebuffer_size_callback(GLFWwindow *win, int w, int h)
{
    glViewport(0,0,w,h);
    win_width = w;
    win_height = h;
    aspect_ratio = (float)w/h;
    //re-setup the auxiliary framebuffer (re-create the texture and renderbuffer with new size)
    setup_framebuffer(w,h);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); //anti aliasing
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    GLFWwindow *win = glfwCreateWindow(800, 600, "Demo app", NULL, NULL);
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

    //IMGUI_CHECKVERSION();
    //ImGui::CreateContext();
    //ImPlot::CreateContext(); //strictly AFTER Imgui::CreateContext();
    //ImGuiIO &io = ImGui::GetIO();
    //io.IniFilename = NULL;
    //io.Fonts->AddFontFromFileTTF("../fonts/arial.ttf", 15.0f);
    //(void)io;
    //ImGui::StyleColorsDark();
    //ImGui_ImplGlfw_InitForOpenGL(win, true);
    //ImGui_ImplOpenGL3_Init("#version 330");

    ///////////////////////////////////////////////////////////////////////////////////

    //ImGuiStyle &imstyle = ImGui::GetStyle();
    //imstyle.WindowMinSize = ImVec2(300.0f,300.0f);

    glfwSetWindowSizeLimits(win, 400, 400, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);
    glfwSetCursorPosCallback(win, cursor_pos_callback);
    glfwSetScrollCallback(win, scroll_callback);
    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwGetWindowSize(win, &win_width, &win_height);

    aspect_ratio = (float)win_width/win_height;
    xpos_previous = win_width/2.0f;
    ypos_previous = win_height/2.0f;

    //Pre-load all the models at once before the while() loop.
    //meshvfn didymain("../obj/vfn/didymos/didymain2019.obj");           bool show_didymain    = false;
    //meshvfn dimorphos("../obj/vfn/didymos/dimorphos_ellipsoid.obj");   bool show_dimorphos   = false;
    //meshvfn bennu("../obj/vfn/bennu196k.obj");                         bool show_bennu       = false;
    //meshvfn eros("../obj/vfn/eros196k.obj");                           bool show_eros        = false;
    //meshvfn gerasimenko("../obj/vfn/gerasimenko256k.obj");             bool show_gerasimenko = false;
    //meshvfn itokawa("../obj/vfn/itokawa196k.obj");                     bool show_itokawa     = false;
    //meshvfn kleopatra("../obj/vfn/kleopatra4k.obj");                   bool show_kleopatra   = false;
    //meshvfn ryugu("../obj/vfn/ryugu196k.obj");                         bool show_ryugu       = false;
    //meshvfn toutatis("../obj/vfn/toutatis3k_radar.obj");               bool show_toutatis    = false;
    //meshvfn vesta("../obj/vfn/vesta256k.obj");                         bool show_vesta       = false;

    //We use only 1 shader throughout the whole app, so we construct it and .use() it here, before the while() loop.
    shader shad("../shaders/vertex/trans_mvpn.vert","../shaders/fragment/lightcurve.frag");
    shad.use();

    //Light calculation parameters (constant).
    glm::vec3 light_dir = glm::vec3(-1.0f,0.0f,0.0f); //Light direction in world coordinates.
    glm::vec3 light_col = glm::vec3(1.0f,1.0f,1.0f); //light color (white always).
    //Now inform the shader about the light params. Since they are constant, we do it only once, here, before the while() loop.
    shad.set_vec3_uniform("light_dir", light_dir);
    shad.set_vec3_uniform("light_col", light_col);

    //projection, view and model matrices. They will change in the while() loop at every frame because the camera will move, so
    //we just declare the variables for now.
    glm::mat4 projection, view, model;

    //Create the auxiliary framebuffer.
    setup_framebuffer(win_width, win_height);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f,0.0f,0.0f,1.0f);

    //Actual lightcure data.
    std::vector<float> time_vector;
    std::vector<float> brightness_vector;
    //bool show_realtime_lightcurve = false;

    t_init = 0.0f;
    while (!glfwWindowShouldClose(win))
    {
        //This t_now value will be used for all time calculations at this frame.
        t_now = (float)glfwGetTime();
        delta_time = t_now - t_init;
        t_init = t_now;

        raw_hardware_input(win);

        //Now we do the following :
        //1) We render the whole scene in the auxiliary framebuffer and then we calculate
        //   the lightcurve. This framebuffer is invisble to us. It will not be rendered in the monitor.
        //2) We render again the same scene in the default framebuffer, just to visualize it interactively.

        //Bind the hidden framebuffer and render the scene there.
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        projection = glm::perspective(glm::radians(fov), aspect_ratio, 0.01f, 5000.0f);
        view = cam.view();
        model = glm::rotate(glm::mat4(1.0f), (float)(10*t_now*pi/180.0f), glm::vec3(0.0f,0.0f,1.0f));
        shad.set_mat4_uniform("projection", projection);
        shad.set_mat4_uniform("view", view);
        shad.set_mat4_uniform("model", model);
        dimorphos.draw_triangles();
        //The scene is now rendered in the auxiliary (hidden) framebuffer. This will not be displayed on the monitor.
        
        //With that scene rendered, let's start calculate the lightcurve :
        time_vector.push_back(t_now);
        brightness_vector.push_back(calculate_brightness(rendered_texture, win_width, win_height));
        //The calculation of the lightvurve is over for this frame. You may write the data in a file and 'continue' the loop.
        
        //What comes next is just for visualization...

        //We want to display the scene in the monitor as well (the default framebuffer).
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        dimorphos.draw_triangles();
        //Done.
        //Pay attention to the fact that we DON'T need to recalculate any variable that will pass to the shaders (matrices, colors, etc...).
        //Those are still in the memory from the previous framebuffer calculations, so we just re-use them.

        //ImGui_ImplOpenGL3_NewFrame();
        //ImGui_ImplGlfw_NewFrame();
        //ImGui::NewFrame();

        //ImGui::Begin("Asteroid model");

        //ImGui::Checkbox("Didymain", &show_didymain);
        //ImGui::Checkbox("Dimorphos", &show_dimorphos);
        //ImGui::Checkbox("Bennu", &show_bennu);
        //ImGui::Checkbox("Eros", &show_eros);
        //ImGui::Checkbox("Gerasimenko", &show_gerasimenko);
        //ImGui::Checkbox("Itokawa", &show_itokawa);
        //ImGui::Checkbox("Kleopatra", &show_kleopatra);
        //ImGui::Checkbox("Ryugu", &show_ryugu);
        //ImGui::Checkbox("Toutatis", &show_toutatis);
        //ImGui::Checkbox("Vesta", &show_vesta);

        //ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); //red color for the plot curves
        
        //if (show_realtime_lightcurve)
        //{
        //    ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y), ImGuiCond_FirstUseEver);
        //    ImGui::Begin("Real-time lightcurve", &show_realtime_lightcurve);
        //    ImVec2 plot_win_size = ImVec2(ImGui::GetWindowSize().x - 20.0f, ImGui::GetWindowSize().y - 40.0f);
        //    if (ImPlot::BeginPlot("brightness(t)", plot_win_size))
        //    {
        //        ImPlot::SetupAxes("Time (t) [sec]", "brightness(t) [ ]");
        //        //ImPlot::SetupAxisLimits(ImAxis_X1, start_time, current_time, ImGuiCond_Always); // Scroll with time
        //        ImPlot::PlotLine("", current_time.data(), lightcurve.data(), current_time.size());
        //        ImPlot::EndPlot();
        //    }
        //    ImGui::End();
        //}

        //ImPlot::PopStyleColor();
        
        //ImGui::End();

        //ImGui::Render();
        //ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
