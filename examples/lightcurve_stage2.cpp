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
unsigned int framebuffer, rbo, rendered_texture;

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

//Handle raw keyboard input (not a callback).
void raw_keyboard_input(GLFWwindow *win)
{
    if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(win, true);
}

//Framebuffer callback. That is, when the window is resized, the following commands will execute.
void framebuffer_size_callback(GLFWwindow *win, int w, int h)
{
    glViewport(0,0,w,h);
    win_width = w;
    win_height = h;
    aspect_ratio = (float)w/h;
    setup_framebuffer(w,h); //Re-setup the auxiliary framebuffer (re-create the texture and renderbuffer with new size)
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext(); //strictly AFTER Imgui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = NULL;
    io.Fonts->AddFontFromFileTTF("../fonts/Arial.ttf", 15.0f);
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(win, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGuiStyle &imstyle = ImGui::GetStyle();
    imstyle.WindowMinSize = ImVec2(200.0f,200.0f);
    imstyle.FrameRounding = 5.0f;
    imstyle.WindowRounding = 5.0f;

    ///////////////////////////////////////////////////////////////////////////////////

    glfwSetWindowSizeLimits(win, 400, 400, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);
    
    glfwGetWindowSize(win, &win_width, &win_height);
    aspect_ratio = (float)win_width/win_height;

    //Pre-load all the meshes, but don't render anything initially.
    meshvfn gerasimenko("../obj/vfn/gerasimenko256k.obj");         bool show_gerasimenko = false;
    meshvfn bennu("../obj/vfn/bennu196k.obj");                     bool show_bennu       = false;
    meshvfn didymain("../obj/vfn/didymos/didymain2019.obj");       bool show_didymain    = false;
    meshvfn itokawa("../obj/vfn/itokawa196k.obj");                 bool show_itokawa     = false;
    meshvfn ryugu("../obj/vfn/ryugu196k.obj");                     bool show_ryugu       = false;
    meshvfn toutatis("../obj/vfn/toutatis3k_radar.obj");           bool show_toutatis    = false;
    meshvfn eros("../obj/vfn/eros196k.obj");                       bool show_eros        = false;
    meshvfn kleopatra("../obj/vfn/kleopatra4k.obj");               bool show_kleopatra   = false;
    bool show_any_asteroid = false;

    //We use only 1 shader throughout the whole app, so we construct it and .use() it here, before the while() loop.
    shader shad("../shaders/vertex/trans_mvpn.vert","../shaders/fragment/dir_light_d.frag");
    shad.use();

    //Light calculation parameters.
    float angle = 180.0f;
    glm::vec3 light_dir = glm::vec3(cos(angle*pi/180.0f),0.0f,sin(angle*pi/180.0f)); //Light direction in world coordinates.
    glm::vec3 light_col = glm::vec3(1.0f,1.0f,1.0f); //light color (grayscale, constant).
    //Camera parameters.
    glm::vec3 cam_pos = glm::vec3(0.0f,-10.0f,0.0f);
    glm::vec3 cam_aim = glm::vec3(0.0f,0.0f,0.0f);
    glm::vec3 cam_up = glm::vec3(0.0f,0.0f,1.0f);
    //projection, view and model matrices.
    float fov = 45.0f;
    glm::mat4 projection = glm::perspective(glm::radians(fov), (float)win_width/win_height, 0.01f,3000.0f);
    glm::mat4 view = glm::lookAt(cam_pos, cam_aim, cam_up);
    glm::mat4 model = glm::mat4(1.0f);
    //Now inform the shader about the constants. Since they are constant, we do it only once, here, before the while() loop.
    glm::vec3 mesh_col = glm::vec3(1.0f,1.0f,1.0f);
    shad.set_vec3_uniform("light_dir", light_dir);
    shad.set_vec3_uniform("light_col", light_col);
    shad.set_vec3_uniform("mesh_col", mesh_col);
    shad.set_mat4_uniform("projection", projection);
    shad.set_mat4_uniform("view", view);
    shad.set_mat4_uniform("model", model);

    //Create the auxiliary framebuffer.
    setup_framebuffer(win_width, win_height);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f,0.0f,0.0f,1.0f);

    //Actual lightcure data.
    std::vector<float> time_vector;
    std::vector<float> brightness_vector;
    const size_t max_size = 4000; //Set the maximum number of lightcurve points to retain.
    bool show_realtime_lightcurve = false;

    float t_delay_due_to_initializations = (float)glfwGetTime();
    while (!glfwWindowShouldClose(win))
    {
        //This t_now value will be used for all time calculations at this frame.
        float t_now = (float)glfwGetTime() - t_delay_due_to_initializations;

        raw_keyboard_input(win);

        //Now we do the following :
        //1) We render the whole scene in the auxiliary framebuffer and then we calculate
        //   the lightcurve. This framebuffer is invisble to us. It will not be rendered in the monitor.
        //2) We render again the same scene in the default framebuffer, just to visualize it interactively.

        if (show_any_asteroid)
        {
            //Bind the hidden framebuffer and render the scene there.
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            model = glm::rotate(glm::mat4(1.0f), (float)(20.0f*t_now*pi/180.0f), glm::vec3(0.0f,0.0f,1.0f));
            shad.set_mat4_uniform("model", model);
            if (show_gerasimenko)
                gerasimenko.draw_triangles();
            else if (show_bennu)
                bennu.draw_triangles();
            else if (show_didymain)
                didymain.draw_triangles();
            else if (show_itokawa)
                itokawa.draw_triangles();
            else if (show_ryugu)
                ryugu.draw_triangles();
            else if (show_toutatis)
                toutatis.draw_triangles();
            else if (show_eros)
                eros.draw_triangles();
            else if (show_kleopatra)
                kleopatra.draw_triangles();

            //The scene is now rendered in the auxiliary (hidden) framebuffer. This will not be displayed on the monitor.

            //With that scene rendered, let's calculate the lightcurve data :
            time_vector.push_back(t_now);
            brightness_vector.push_back(calculate_brightness(rendered_texture, win_width, win_height));
            //The calculation of the lightvurve is over for this frame. You may write the vector data in a file and 'continue' the loop.
                
            //What comes next is just for visualization...

            //We want to display the scene in the monitor as well (the default framebuffer).
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            if (show_gerasimenko)
                gerasimenko.draw_triangles();
            else if (show_bennu)
                bennu.draw_triangles();
            else if (show_didymain)
                didymain.draw_triangles();
            else if (show_itokawa)
                itokawa.draw_triangles();
            else if (show_ryugu)
                ryugu.draw_triangles();
            else if (show_toutatis)
                toutatis.draw_triangles();
            else if (show_eros)
                eros.draw_triangles();
            else if (show_kleopatra)
                kleopatra.draw_triangles();
        }
        else
        {
            time_vector.push_back(t_now);
            brightness_vector.push_back(0.0f);
        }

        if (time_vector.size() > max_size)
        {
            time_vector.erase(time_vector.begin());
            brightness_vector.erase(brightness_vector.begin());
        }

        //Done.
        //Pay attention to the fact that we DON'T need to recalculate any uniform variables that go through the shaders (matrices, colors, etc...).
        //Those are still in the memory from the previous framebuffer calculations, so we just re-use them.

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0.0f, ImGui::GetIO().DisplaySize.y), ImGuiCond_FirstUseEver, ImVec2(0.0f, 1.0f));
        ImGui::Begin("Menu");

        ImGui::Text("Light direction");
        ImGui::SliderFloat("[deg]##angle", &angle, 0.0f, 360.0f);
        ImGui::Separator();
        light_dir.x = cos(angle*pi/180.0f);
        light_dir.z = sin(angle*pi/180.0f);
        shad.set_vec3_uniform("light_dir", light_dir);

        ImGui::Text("Camera distance");
        ImGui::SliderFloat("[km]", &cam_pos.y, -250.0f,-1.0f);
        ImGui::Separator();
        shad.set_vec3_uniform("cam_pos", cam_pos);
        view = glm::lookAt(cam_pos, cam_aim, cam_up);
        shad.set_mat4_uniform("view", view);

        ImGui::Text("Camera f.o.v.");
        ImGui::SliderFloat("[deg]##fov", &fov, 1.0f, 45.0f);
        ImGui::Separator();
        projection = glm::perspective(glm::radians(fov), aspect_ratio, 0.01f, 3000.0f);
        shad.set_mat4_uniform("projection", projection);

        //Which asteroid
        if (ImGui::Checkbox("Gerasimenko", &show_gerasimenko))
        {
            show_any_asteroid = true;
            show_bennu = false;
            show_didymain = false;
            show_itokawa = false;
            show_ryugu = false;
            show_toutatis = false;
            show_eros = false;
            show_kleopatra = false;
        }
        if (ImGui::Checkbox("Bennu", &show_bennu))
        {
            show_any_asteroid = true;
            show_gerasimenko = false;
            show_didymain = false;
            show_itokawa = false;
            show_ryugu = false;
            show_toutatis = false;
            show_eros = false;
            show_kleopatra = false;
        }
        if (ImGui::Checkbox("Didymain", &show_didymain))
        {
            show_any_asteroid = true;
            show_gerasimenko = false;
            show_bennu = false;
            show_itokawa = false;
            show_ryugu = false;
            show_toutatis = false;
            show_eros = false;
            show_kleopatra = false;
        }
        if (ImGui::Checkbox("Itokawa", &show_itokawa))
        {
            show_any_asteroid = true;
            show_gerasimenko = false;
            show_bennu = false;
            show_didymain = false;
            show_ryugu = false;
            show_toutatis = false;
            show_eros = false;
            show_kleopatra = false;
        }
        if (ImGui::Checkbox("Ryugu", &show_ryugu))
        {
            show_any_asteroid = true;
            show_gerasimenko = false;
            show_bennu = false;
            show_didymain = false;
            show_itokawa = false;
            show_toutatis = false;
            show_eros = false;
            show_kleopatra = false;
        }
        if (ImGui::Checkbox("Toutatis", &show_toutatis))
        {
            show_any_asteroid = true;
            show_gerasimenko = false;
            show_bennu = false;
            show_didymain = false;
            show_itokawa = false;
            show_ryugu = false;
            show_eros = false;
            show_kleopatra = false;
        }
        if (ImGui::Checkbox("Eros", &show_eros))
        {
            show_any_asteroid = true;
            show_gerasimenko = false;
            show_bennu = false;
            show_didymain = false;
            show_itokawa = false;
            show_ryugu = false;
            show_toutatis = false;
            show_kleopatra = false;
        }
        if (ImGui::Checkbox("Kleopatra", &show_kleopatra))
        {
            show_any_asteroid = true;
            show_gerasimenko = false;
            show_bennu = false;
            show_didymain = false;
            show_itokawa = false;
            show_ryugu = false;
            show_toutatis = false;
            show_eros = false;
        }

        if (!show_gerasimenko && !show_bennu && !show_didymain && !show_itokawa && !show_ryugu && !show_toutatis && !show_eros && !show_kleopatra)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            show_any_asteroid = false;
        }

        ImGui::Dummy(ImVec2(0.0f,15.0f));

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.0f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        if (ImGui::Button("Lightcurve", ImVec2(70.0f,50.0f)))
        {
            show_realtime_lightcurve = !show_realtime_lightcurve;
        }
        ImGui::PopStyleColor(3);

        if (show_realtime_lightcurve)
        {
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(5.0f*ImGui::GetWindowSize().x, ImGui::GetWindowSize().y), ImGuiCond_FirstUseEver);
            ImGui::Begin("Real-time lightcurve", &show_realtime_lightcurve);
            ImVec2 plot_win_size = ImVec2(ImGui::GetWindowSize().x - 20.0f, ImGui::GetWindowSize().y - 40.0f);
            if (ImPlot::BeginPlot("Lightcurve", plot_win_size))
            {
                ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); //red color for the plot curves
                ImPlot::SetupAxes("Time [sec]", "Brightness [norm]");
                ImPlot::SetupAxisLimits(ImAxis_X1, t_now-70.0f, t_now, ImGuiCond_Always); // Scroll with time
                ImPlot::PlotLine("", time_vector.data(), brightness_vector.data(), time_vector.size());
                ImPlot::PopStyleColor();
                ImPlot::EndPlot();
            }
            ImGui::End();
        }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}
