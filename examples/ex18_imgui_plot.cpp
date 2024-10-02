#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"
#include"../imgui/implot.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include<cstdio>
#include<cmath>
#include<vector>

int win_width = 1000, win_height = 800;

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    win_width = width;
    win_height = height;
    glViewport(0,0,width,height);
}

int main()
{
	glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    GLFWwindow *window = glfwCreateWindow(win_width, win_height, "GUI plots", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);
    
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext(); //Strictly AFTER Imgui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = NULL;
    io.Fonts->AddFontFromFileTTF("../fonts/Arial.ttf", 15.0f);
    (void)io;
    ImGui::StyleColorsLight();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGuiStyle &imstyle = ImGui::GetStyle();
    imstyle.WindowMinSize = ImVec2(300.0f,300.0f);
    imstyle.FrameRounding = 5.0f;
    imstyle.WindowRounding = 5.0f;

    //Static curves.
    double x[500], sinx[500], lnx[500];
    bool show_sinx_plot, show_lnx_plot;
    for (int i = 0; i < 500; ++i)
    {
        x[i] = 0.1*i;
        sinx[i] = sin(x[i]);
        lnx[i] = log(x[i]);
    }
    show_sinx_plot = show_lnx_plot = false;
    
    //Real time standing wave curve.
    double standing_wave_xt[500]; //This will be dynamically updated in the game loop because we need the time.
    bool show_standing_wave_plot = false;

    //Real time beat curve.
    std::vector<double> time_data;
    std::vector<double> beat_data;
    const size_t max_size_data = 11000; //Set the maximum number of curve points to retain.
    bool show_beat_plot = false;

    //double t_until_while_loop = glfwGetTime();
    // Time window to display (e.g., show only the last 10 seconds of data)
    //float time_window = 10.0f;

    glClearColor(0.0f,0.2f,0.2f,1.0f);
    while (!glfwWindowShouldClose(window))
    {
        double tnow = glfwGetTime();

		glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        static bool popen = true;
        ImGui::Begin("Menu", &popen);
        if (!popen)
            glfwSetWindowShouldClose(window, true);

        ImGui::Checkbox("Click to plot sin(x)", &show_sinx_plot);
        ImGui::Checkbox("Click to plot ln(x)", &show_lnx_plot);
        ImGui::Checkbox("Click to plot standing_wave(x,t)", &show_standing_wave_plot);
        ImGui::Checkbox("Click to plot beat", &show_beat_plot);
        
        //Static curve sin(x).
        if (show_sinx_plot)
        {
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y), ImGuiCond_FirstUseEver);
            ImGui::Begin("sin(x) plot", &show_sinx_plot);
            ImVec2 plot_win_size = ImVec2(ImGui::GetWindowSize().x - 20.0f, ImGui::GetWindowSize().y - 40.0f);
            if (ImPlot::BeginPlot("y = sin(x)", plot_win_size))
            {
                ImPlot::SetupAxes("x","y");
                ImPlot::PlotLine("", x,sinx,500);
                ImPlot::EndPlot();  
            }
            ImGui::End();
        }

        //Static curve ln(x).
        if (show_lnx_plot)
        {
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y), ImGuiCond_FirstUseEver);
            ImGui::Begin("ln(x) plot", &show_lnx_plot);
            ImVec2 plot_win_size = ImVec2(ImGui::GetWindowSize().x - 20.0f, ImGui::GetWindowSize().y - 40.0f);
            if (ImPlot::BeginPlot("y = ln(x)", plot_win_size))
            {
                ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                ImPlot::SetupAxes("x","y");
                ImPlot::PlotLine("", x,lnx,500);
                ImPlot::PopStyleColor();
                ImPlot::EndPlot();  
            }
            ImGui::End();
        }

        //Standing wave curve y-component evaluation (which depends on time).
        for (int i = 0; i < 500; ++i)
            standing_wave_xt[i] = 0.5*cos(x[i])*cos(2.0*ImGui::GetTime());
        //To be honest, the part 0.5*cos(x[i]) does not dependend on time and hence, it could be precomputed outside the while().
        if (show_standing_wave_plot)
        {
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y), ImGuiCond_FirstUseEver);
            ImGui::Begin("Standing wave plot", &show_standing_wave_plot);
            ImVec2 plot_win_size = ImVec2(ImGui::GetWindowSize().x - 20.0f, ImGui::GetWindowSize().y - 40.0f);
            if (ImPlot::BeginPlot("y = 0.5*cos(x)*cos(2*t)", plot_win_size))
            {
                ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.0f, 0.5f, 0.0f, 1.0f));
                ImPlot::SetupAxes("x","y");
                ImPlot::PlotLine("", x,standing_wave_xt,500);
                ImPlot::PopStyleColor();
                ImPlot::EndPlot();  
            }
            ImGui::End();
        }

        //When the vectors reach the full capacity (max_size_data), then for every new data point entering
        //the vectors, the 'firstest' one shall be deleted.
        time_data.push_back(tnow);
        beat_data.push_back(sin(2.0*tnow) + sin(2.3*tnow));
        if (time_data.size() > max_size_data)
        {
            time_data.erase(time_data.begin());
            beat_data.erase(beat_data.begin());
        }
        //Beat curve y-component evaluation (which depends on time).
        if (show_beat_plot)
        {
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y), ImGuiCond_FirstUseEver);
            ImGui::Begin("Beat plot", &show_beat_plot);
            ImVec2 plot_win_size = ImVec2(ImGui::GetWindowSize().x - 20.0f, ImGui::GetWindowSize().y - 40.0f);
            if (ImPlot::BeginPlot("y = sin(2*t) + sin(2.3*t)", plot_win_size))
            {
                ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
                ImPlot::SetupAxes("t [sec]", "y");
                ImPlot::SetupAxisLimits(ImAxis_X1, tnow - 70.0, tnow, ImGuiCond_Always); //Automatically scroll the view with time.
                ImPlot::PlotLine("", time_data.data(), beat_data.data(), time_data.size());
                ImPlot::PopStyleColor();
                ImPlot::EndPlot();
            }
            ImGui::End();
        }
        
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext(); //Strictly BEFORE Imgui::DestroyContext();
    ImGui::DestroyContext();

    glfwTerminate();

	return 0;
}