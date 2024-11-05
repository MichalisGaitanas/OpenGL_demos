#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include<cstdio>
#include<cmath>

#include<thread>
#include<atomic>

int win_width = 800, win_height = 600;

//Flag to indicate if func is running. DON'T use simple boolean variable for this job!
//Atomic bool will prevent race conditions, whereas simple bool will not.
std::atomic<bool> is_func_running(false);

//Atomic float to track progress of func.
std::atomic<float> progress(0.0f);

void key_callback(GLFWwindow *window, int key, int /*scancode*/, int action, int /*mods*/)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow *, int w, int h)
{
    if (w < 1) w = 1;
    if (h < 1) h = 1;
    win_width = w;
    win_height = h;
    glViewport(0,0,w,h);
}

void func()
{
    is_func_running = true;
    progress = 0.0f;
    for (float z = 0.0f; z <= 20000.0f; z += 0.001f)
    {
        (void)exp(sin(sqrt(z*fabs(z)+ cos(z))));
        progress = z/20000.0f; //Update progress.
    }
    is_func_running = false;
}

void glfw_center_window(GLFWwindow *win)
{
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    glfwGetWindowSize(win, &win_width, &win_height);
    int centx = (mode->width - win_width)/2;
    int centy = (mode->height - win_height)/2;
    glfwSetWindowPos(win, centx, centy);
}

int main()
{
	glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow *window = glfwCreateWindow(win_width, win_height, "GUI threading", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);
    glfw_center_window(window);
    glfwSetKeyCallback(window, key_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }

    //Setup ImGui.
	IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = NULL;
    io.Fonts->AddFontFromFileTTF("../fonts/Arial.ttf", 15.0f);
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    std::thread func_thread;

    glClearColor(0.1f,0.1f,0.1f, 1.0f);
    while (!glfwWindowShouldClose(window))
    {
		glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(300.0f,300.0f), ImGuiCond_FirstUseEver); 
		ImGui::Begin("Menu");

        if(ImGui::Button("Button 1"))
            func();

        if (ImGui::Button("Button 2") && !is_func_running)
        {
            func_thread = std::thread(func);
            func_thread.detach();
        }

        ImGui::SameLine();
        if (!is_func_running)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,0,0,255));
            ImGui::Text("func() is not running.");
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0,255,0,255));
            ImGui::Text("func() is running...");
            ImGui::PopStyleColor();
        }

        //Display progressbar.
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f,0.7f,0.0f, 1.0f)); //Green.
        ImGui::ProgressBar(progress, ImVec2(150.0f,20.0f));
        ImGui::PopStyleColor();
        ImGui::SameLine();
        if(ImGui::Button("Reset"))
            progress = 0.0f;

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