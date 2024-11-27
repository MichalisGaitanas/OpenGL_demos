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

#include"../include/shader.h"
#include"../include/mesh.h"

#ifdef _OPENMP
#include<omp.h>
#endif

int win_width = 800, win_height = 600; //Initial glfw window size.
unsigned int fbo, rbo, tex; //Framebuffer object, renderbuffer object and texture ID.

void omp_setup_threads()
{
#ifdef _OPENMP
    omp_set_dynamic(false); //Obey to my following thread number request.
    omp_set_num_threads(omp_get_max_threads()/2); //Occupy half of the machine's threads for the calculation of the lightcurve at each frame.
#endif
}

//Calculate brightness (lightcurve) from the rendered scene in the hidden framebuffer (fbo).
float get_brightness(unsigned int tex, int width_pix, int height_pix)
{
    glBindTexture(GL_TEXTURE_2D, tex);
    std::vector<float> pixels(width_pix*height_pix);
    
    //Read the pixels from the texture (only the red channel, i.e. grayscale color).
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, pixels.data());
    
    //Sum up the intensity values stored in the red channel.
#ifdef _OPENMP
    int i, total_pixels = width_pix*height_pix;
    float brightness = 0.0f;
    #pragma omp parallel for firstprivate(total_pixels)\
                             private(i)\
                             shared(pixels)\
                             schedule(static)\
                             reduction(+:brightness)
    for (i = 0; i < total_pixels; ++i)
        brightness += pixels[i];
#else
    float brightness = 0.0f;
    for (int i = 0; i < width_pix*height_pix; ++i)
        brightness += pixels[i];
#endif

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
    omp_setup_threads();

    //Setup glfw stuff.
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    GLFWwindow *win = glfwCreateWindow(win_width, win_height, "Lightcurve generation (no shadow)", NULL, NULL);
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
    ImGui_ImplOpenGL3_Init("#version 450");
    ImGuiStyle &imstyle = ImGui::GetStyle();
    imstyle.WindowMinSize = ImVec2(200.0f,200.0f);
    imstyle.FrameRounding = 5.0f;
    imstyle.WindowRounding = 5.0f;

    //Pre-load all the asteroid meshes. All are exposed in the gui.
    meshvfn gerasimenko("../obj/vfn/asteroids/gerasimenko256k.obj");      bool show_gerasimenko = false;
    meshvfn bennu("../obj/vfn/asteroids/bennu196k.obj");                  bool show_bennu       = false;
    meshvfn didymain("../obj/vfn/asteroids/didymos/didymain2019.obj");    bool show_didymain    = false;
    meshvfn itokawa("../obj/vfn/asteroids/itokawa196k.obj");              bool show_itokawa     = false;
    meshvfn ryugu("../obj/vfn/asteroids/ryugu196k.obj");                  bool show_ryugu       = false;
    meshvfn toutatis("../obj/vfn/asteroids/toutatis3k_radar.obj");        bool show_toutatis    = false;
    meshvfn eros("../obj/vfn/asteroids/eros196k.obj");                    bool show_eros        = false;
    meshvfn kleopatra("../obj/vfn/asteroids/kleopatra4k.obj");            bool show_kleopatra   = false;
    //Btw, it could be done with an array and then looooop, but whatever... I got bored.

    //We use only 1 shader throughout the whole app, so we construct it and .use() it here, before the while() loop.
    shader shad("../shaders/vertex/trans_mvpn.vert","../shaders/fragment/dir_light_d.frag");
    shad.use();

    //Light calculation parameters.
    float angle = 180.0f; //Exposed in the gui.
    glm::vec3 light_dir = glm::vec3(cos(glm::radians(angle)), 0.0f, sin(glm::radians(angle))); //Light direction in 'world' coordinates.
    glm::vec3 light_col = glm::vec3(1.0f,1.0f,1.0f); //Light color. Grayscale (white to black).
    
    glm::vec3 mesh_col = glm::vec3(1.0f,1.0f,1.0f); //Asteroid color (white).

    //Camera parameters.
    glm::vec3 cam_pos = glm::vec3(0.0f,-10.0f,0.0f); //Camera's position in 'world' coordinates. Exposed in the gui.
    glm::vec3 cam_aim = glm::vec3(0.0f,0.0f,0.0f); //Camera aims at the origin of our 'world' coordsys.
    glm::vec3 cam_up = glm::vec3(0.0f,0.0f,1.0f); //Camera's local up direction vector (+z).
    float fov = 45.0f; //Camera's field of view. Exposed in the gui.

    //Actual lightcure data.
    std::vector<float> time_vector;
    std::vector<float> brightness_vector;
    const size_t max_size = 4000; //Set the maximum number of lightcurve points to retain.
    bool show_realtime_lightcurve = false; //Exposed in the gui.

    //Transformation matrices that take us from the 3D 'world' coordsys to the 2D monitor pixeled coordsys.
    glm::mat4 projection, view, model;
    //Imagine a point (vertex) in the 3D space with position vector v = [x,y,z] with respect to what we call 'world' origin.
    //model*v transforms v into v' = [x',y',z'], depending on the model matrix content (translation, rotation, scale).
    //view*v' transforms v' into v'' = [x'',y'',z'']. This transformation is applied because we want to view v' from the camera's perspective.
    //projection*v'' transforms v'' into v''' = [x''',y''',z''']. This transformation transforms the (with respect to the camera) scene to clip space (still 3D).
    //This final transformation (in the case of perspective projection) makes distant objects smaller and near objects larger, mimicking how we perceive the real world.

    //Now inform the shader about the uniforms that are NOT going to change in the game loop.
    shad.set_vec3_uniform("light_col", light_col);
    shad.set_vec3_uniform("mesh_col", mesh_col);

    //Create the (clean) hidden framebuffer.
    setup_fbo(win_width, win_height);

    glEnable(GL_DEPTH_TEST); //Automatic depth test.
    glEnable(GL_CULL_FACE); //Enable face culling.
    glClearColor(0.0f,0.0f,0.0f,1.0f); //Black background color.

    float t_delay_due_to_initializations = (float)glfwGetTime();
    while (!glfwWindowShouldClose(win))
    {
        //This t_now value will be used for all time calculations AT THIS frame.
        float t_now = (float)glfwGetTime() - t_delay_due_to_initializations;

        //Now we do the following :
        //1) We render the whole scene in the hidden framebuffer and then we calculate
        //   the lightcurve. This framebuffer is invisble to us. It will not be rendered in the monitor.
        //2) We render again the same scene in the default framebuffer, just to visualize it interactively.

        projection = glm::perspective(glm::radians(fov), (float)win_width/win_height, 0.01f, 3000.0f);
        shad.set_mat4_uniform("projection", projection);

        view = glm::lookAt(cam_pos, cam_aim, cam_up);
        shad.set_mat4_uniform("view", view);

        time_vector.push_back(t_now);
        if (show_gerasimenko || show_bennu || show_didymain || show_itokawa || show_ryugu || show_toutatis || show_eros || show_kleopatra)
        {
            //Bind the hidden framebuffer and render the scene there.
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            model = glm::rotate(glm::mat4(1.0f), glm::radians(20.0f*t_now), glm::vec3(0.0f,0.0f,1.0f));
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

            //The scene is now rendered in the hidden framebuffer. It will not be displayed on the monitor.
            //With that scene rendered, let's calculate the lightcurve data :
            brightness_vector.push_back(get_brightness(tex, win_width, win_height));
            //The calculation of the lightvurve is over for this frame. The 'backend' hidden framebuffer holds the important info.

            //If we want to display the scene in the monitor as well (the default framebuffer), then :
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
        else //This means that no asteroid is chosen, hence total blackness in the framebuffer, hence zerooooooo.
            brightness_vector.push_back(0.0f);

        //Prevent the data will not grow unstopably. When the vectors reach the desired capacity (max_size),
        //then for every new data point entering the vectors, the 'firstest' one shall be deleted, maintaining a constant size.
        if (time_vector.size() > max_size)
        {
            time_vector.erase(time_vector.begin());
            brightness_vector.erase(brightness_vector.begin());
        }

        //Render gui stuff.

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0.0f, ImGui::GetIO().DisplaySize.y), ImGuiCond_FirstUseEver, ImVec2(0.0f, 1.0f));
        ImGui::Begin("Menu");

        ImGui::Text("Light direction (plane x-z)");
        ImGui::SliderFloat("[deg]##angle", &angle, 0.0f, 360.0f);
        ImGui::Separator();
        light_dir.x = cos(glm::radians(angle));
        light_dir.z = sin(glm::radians(angle));
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

        //Choose asteroid algorithm.
        ImGui::Text("Asteroids");
        if (ImGui::Checkbox("Gerasimenko", &show_gerasimenko))
            show_bennu = show_didymain = show_itokawa = show_ryugu = show_toutatis = show_eros = show_kleopatra = false;
        if (ImGui::Checkbox("Bennu", &show_bennu))
            show_gerasimenko = show_didymain = show_itokawa = show_ryugu = show_toutatis = show_eros = show_kleopatra = false;
        if (ImGui::Checkbox("Didymain", &show_didymain))
            show_gerasimenko = show_bennu = show_itokawa = show_ryugu = show_toutatis = show_eros = show_kleopatra = false;
        if (ImGui::Checkbox("Itokawa", &show_itokawa))
            show_gerasimenko = show_bennu = show_didymain = show_ryugu = show_toutatis = show_eros = show_kleopatra = false;
        if (ImGui::Checkbox("Ryugu", &show_ryugu))
            show_gerasimenko = show_bennu = show_didymain = show_itokawa = show_toutatis = show_eros = show_kleopatra = false;
        if (ImGui::Checkbox("Toutatis", &show_toutatis))
            show_gerasimenko = show_bennu = show_didymain = show_itokawa = show_ryugu = show_eros = show_kleopatra = false;
        if (ImGui::Checkbox("Eros", &show_eros))
            show_gerasimenko = show_bennu = show_didymain = show_itokawa = show_ryugu = show_toutatis = show_kleopatra = false;
        if (ImGui::Checkbox("Kleopatra", &show_kleopatra))
            show_gerasimenko = show_bennu = show_didymain = show_itokawa = show_ryugu = show_toutatis = show_eros = false;
        ImGui::Separator();

        if (!show_gerasimenko && !show_bennu && !show_didymain && !show_itokawa && !show_ryugu && !show_toutatis && !show_eros && !show_kleopatra)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        ImGui::Text("Performance ");
        ImGui::Text("FPS [ %.0f ] ", ImGui::GetIO().Framerate);
        ImGui::Separator();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.0f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        if (ImGui::Button("Lightcurve", ImVec2(70.0f,30.0f)))
            show_realtime_lightcurve = !show_realtime_lightcurve;
        ImGui::PopStyleColor(3);

        if (show_realtime_lightcurve)
        {
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(5.0f*ImGui::GetWindowSize().x, ImGui::GetWindowSize().y), ImGuiCond_FirstUseEver);
            ImGui::Begin("Real-time lightcurve", &show_realtime_lightcurve);
            ImVec2 plot_win_size = ImVec2(ImGui::GetWindowSize().x - 20.0f, ImGui::GetWindowSize().y - 40.0f);
            if (ImPlot::BeginPlot("Lightcurve", plot_win_size))
            {
                ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.75f, 0.0f, 0.0f, 1.0f)); //Red color for the lightcurve.
                ImPlot::SetupAxes("Time [sec]", "Brightness [norm]");
                ImPlot::SetupAxisLimits(ImAxis_X1, t_now-70.0f, t_now, ImGuiCond_Always); //Automatically scroll with time along the t-axis.
                ImPlot::PlotLine("", time_vector.data(), brightness_vector.data(), time_vector.size());
                ImPlot::PopStyleColor();
                ImPlot::EndPlot();
            }
            ImGui::End();
        }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext(); //Strictly BEFORE Imgui::DestroyContext();
    ImGui::DestroyContext();

    glfwTerminate();

    return 0;
}
