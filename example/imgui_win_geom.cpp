
//always include imgui stuff first
#include"imgui.h"
#include"imgui_impl_glfw.h"
#include"imgui_impl_opengl3.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<cstdio>

void raw_hardware_input(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
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

    //initialize imgui assuming glfw will be used
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = NULL;
    (void)io;
    ImGui::StyleColorsDark(); //set background color for imgui window
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");    

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    while (!glfwWindowShouldClose(window))
    {   
		glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        float dispx = ImGui::GetIO().DisplaySize.x;
        float dispy = ImGui::GetIO().DisplaySize.y;

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(dispx/7.0f, dispy), ImGuiCond_FirstUseEver);
        ImGui::Begin("Left window", NULL);
        ImGui::Text("Input, output, etc...");
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(dispx - dispx/7.0f, 0.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(dispx/7.0f, dispy), ImGuiCond_FirstUseEver);
        ImGui::Begin("Right window", NULL);
        ImGui::Text("Graphics controls");
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(dispx/7.0f, dispy - dispy/9.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(dispx - 2*dispx/7.0f, dispy/9.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Bottom window", NULL);
        ImGui::Text("Messages, commands, etc...");
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        raw_hardware_input(window);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //free imgui resources
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();

	return 0;
}