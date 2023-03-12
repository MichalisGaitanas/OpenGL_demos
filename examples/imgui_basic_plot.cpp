#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"
#include"../imgui/implot.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include<cstdio>
#include<cmath>

//Kill the appif 'esc' is pressed.
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    return;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0,0,width,height);
    return;
}

int main()
{
	glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    GLFWwindow *window = glfwCreateWindow(800,600, "imgui", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to open a glfw window. Exiting...\n");
        return 0;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }

    glfwSetKeyCallback(window, key_callback);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext(); //strictly AFTER Imgui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = NULL;
    io.Fonts->AddFontFromFileTTF("../fonts/arial.ttf", 15.0f);
    (void)io;
    ImGui::StyleColorsLight();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    //change the default minimum size of an imgui window
    ImGuiStyle &imstyle = ImGui::GetStyle();
    imstyle.WindowMinSize = ImVec2(300.0f,300.0f);

    //create some data to plot
    double x[500];
    double y1[500], y2[500];
    for (int i = 0; i < 500; i++)
    {
        x[i] = 0.1*i;
        y1[i] = sin(x[i]);
        y2[i] = log(x[i]);
    }

    //checkbutton booleans
    bool show_sinx_plot = false;
    bool show_lnx_plot = false;
    bool show_dynamic_plot = false;

    glClearColor(0.0f, 0.3f, 0.3f, 1.0f);
    while (!glfwWindowShouldClose(window))
    {

		glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Menu imgui window");

        ImGui::Checkbox("Click to plot y = sin(x)", &show_sinx_plot);
        ImGui::Checkbox("Click to plot y = ln(x)", &show_lnx_plot);
        ImGui::Checkbox("Click to plot y = standing_wave(x)", &show_dynamic_plot);
        ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); //red color for the plot curves
        if (show_sinx_plot)
        {
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y), ImGuiCond_FirstUseEver);
            ImGui::Begin("Plot imgui window (sin(x))", &show_sinx_plot);
            ImVec2 plot_win_size = ImVec2(ImGui::GetWindowSize().x - 20.0f, ImGui::GetWindowSize().y - 40.0f);
            if (ImPlot::BeginPlot("y = sin(x)", plot_win_size))
            {
                ImPlot::SetupAxes("x","y");
                ImPlot::PlotLine("", x,y1,500);
                ImPlot::EndPlot();  
            }
            ImGui::End();
        }
        if (show_lnx_plot)
        {
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y), ImGuiCond_FirstUseEver);
            ImGui::Begin("Plot imgui window (ln(x))", &show_lnx_plot);
            ImVec2 plot_win_size = ImVec2(ImGui::GetWindowSize().x - 20.0f, ImGui::GetWindowSize().y - 40.0f);
            if (ImPlot::BeginPlot("y = ln(x)",plot_win_size))
            {
                ImPlot::SetupAxes("x","y");
                ImPlot::PlotLine("", x,y2,500);
                ImPlot::EndPlot();  
            }
            ImGui::End();
        }

        //create a dynamically updated standing wave
        double y3[500];
        for (int i = 0; i < 500; ++i)
            y3[i] = 0.5*cos(x[i])*cos(3.0*ImGui::GetTime());
        if (show_dynamic_plot)
        {
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y), ImGuiCond_FirstUseEver);
            ImGui::Begin("Plot imgui window (0.5*cos(x)*cos(3*t))", &show_dynamic_plot);
            ImVec2 plot_win_size = ImVec2(ImGui::GetWindowSize().x - 20.0f, ImGui::GetWindowSize().y - 40.0f);
            if (ImPlot::BeginPlot("y = standing_wave(x,t)",plot_win_size))
            {
                ImPlot::SetupAxes("x","y");
                ImPlot::PlotLine("", x,y3,500);
                ImPlot::EndPlot();  
            }
            ImGui::End();
        }
        ImPlot::PopStyleColor();
        
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    //free imgui resources
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext(); //strictly BEFORE Imgui::CreateContext();
    ImGui::DestroyContext();

    glfwTerminate();

	return 0;
}