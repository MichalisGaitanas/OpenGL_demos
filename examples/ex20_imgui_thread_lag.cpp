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

void simulation(std::atomic<bool> &is_running, std::atomic<bool> &abort_requested, std::atomic<float> &progress)
{
    is_running.store(true);
    abort_requested.store(false);
    progress.store(0.0f);
    for (float z = 0.0f; z <= 20000.0f; z += 0.001f)
    {
        if (abort_requested.load())
            break;

        (void)exp(sin(sqrt(z*fabs(z) + cos(z))));
        progress.store(z/20000.0f); //Update progress.
    }
    is_running.store(false);
}

void glfw_center_window(GLFWwindow *win)
{
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    glfwGetWindowSize(win, &win_width, &win_height);
    int centx = (mode->width - win_width) / 2;
    int centy = (mode->height - win_height) / 2;
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

    // Setup ImGui.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = NULL;
    io.Fonts->AddFontFromFileTTF("../fonts/Arial.ttf", 15.0f);
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGuiStyle &imstyle = ImGui::GetStyle();
    imstyle.FrameRounding = 6.0f;
    imstyle.WindowRounding = 6.0f;

    //Flags to indicate if simulation() is running and if an abort has been requested.
    std::atomic<bool> is_running(false);
    std::atomic<bool> abort_requested(false);
    std::atomic<float> progress(0.0f); //Atomic float to track progress of simulation().

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(300.0f, 300.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Menu");

        //This button will freeze the execution. However the OS will queue any requests like button presses, etc..
        if (ImGui::Button("Run (lag)", ImVec2(85.0f, 25.0f)))
        {
            simulation(is_running, abort_requested, progress);
            printf("Done.\n");
        }


        if (!is_running.load()) //In this case the simulation is NOT currently running, hence "Run (thread)" can be pressed (to start), but "Abort", cannot be pressed (nothing to abort).
        {
            if (ImGui::Button("Run (thread)", ImVec2(85.0f, 25.0f)))
            {
                std::thread simulation_thread = std::thread(simulation, std::ref(is_running), std::ref(abort_requested), std::ref(progress));
                simulation_thread.detach();
            }
            ImGui::SameLine();
            ImGui::BeginDisabled();
            ImGui::Button("Abort", ImVec2(85.0f, 25.0f));
            ImGui::EndDisabled();
        }
        else //Now the opposite happens. "Run (thread)" is disabled coz the simulation is running and "Abort" is enabled, so that one may stop the running.
        {
            ImGui::BeginDisabled();
            ImGui::Button("Run (thread)", ImVec2(85.0f, 25.0f));
            ImGui::EndDisabled();
            ImGui::SameLine();
            if (ImGui::Button("Abort", ImVec2(85.0f, 25.0f)))
                abort_requested.store(true);
        }

        //Display progress bar with different colors based on 'abort_requested' state.
        if (abort_requested.load())
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.7f, 0.0f, 0.0f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.0f, 0.7f, 0.0f, 1.0f));
        ImGui::ProgressBar(progress, ImVec2(180.0f, 20.0f));
        ImGui::PopStyleColor();

        ImGui::SameLine();
        if (!is_running.load())
        {
            if (ImGui::Button("Reset %"))
                progress.store(0.0f);
        }
        else
        {
            ImGui::BeginDisabled();
            ImGui::Button("Reset %");
            ImGui::EndDisabled();
        }

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
