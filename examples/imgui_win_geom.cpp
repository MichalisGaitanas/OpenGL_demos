#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<cstdio>

class Window
{
    
private:

    GLFWwindow *m_pointer;
    int m_width, m_height;
    float m_aspectratio;

    static void framebuffer_size_callback(GLFWwindow *pointer, int width, int height)
    {
        Window *instance = static_cast<Window*>(glfwGetWindowUserPointer(pointer));
        if (instance != NULL)
        {
            instance->m_width = width;
            instance->m_height = height;
            instance->m_aspectratio = width/(float)height;
            glViewport(0,0, width,height);
        }
        else
            printf("'glfwGetWindowUserPointer(pointer)' is NULL. Exiting framebuffer_size_callback()...\n");

        return;
    }

    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        return;
    }

public:

    Window()
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
        glfwWindowHint(GLFW_REFRESH_RATE, 60);

        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);

        m_width = mode->width;
        m_height = mode->height;

        m_pointer = glfwCreateWindow(m_width, m_height, "KIMIN", NULL, NULL);
        if (m_pointer == NULL)
        {
            printf("Failed to create glfw window. Calling glfwTerminate().\n");
            glfwTerminate();
        }

        glfwSetWindowUserPointer(m_pointer, this);
        glfwMakeContextCurrent(m_pointer);
        glfwSetWindowSizeLimits(m_pointer, 400, 400, GLFW_DONT_CARE, GLFW_DONT_CARE);
        glfwSwapInterval(1);

        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK)
        {
            printf("Failed to initialize glew. Calling glfwTerminate().\n");
            glfwTerminate();
        }

        glfwSetFramebufferSizeCallback(m_pointer, framebuffer_size_callback);
        glfwSetKeyCallback(m_pointer, key_callback);
    }

    ~Window()
    {
        glfwDestroyWindow(m_pointer);
        glfwTerminate();
    }

    void game_loop()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.IniFilename = NULL;
        (void)io;
        ImGui::StyleColorsDark(); //set background color for imgui window
        ImGui_ImplGlfw_InitForOpenGL(m_pointer, true);
        ImGui_ImplOpenGL3_Init("#version 330");    
        
        glClearColor(0.2f,0.2f,0.9f,1.0f);
        while (!glfwWindowShouldClose(m_pointer))
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
            

            glfwSwapBuffers(m_pointer);
            glfwPollEvents();
        }
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        return;
    }
};

int main()
{
    Window window;
    window.game_loop();

    return 0;
}