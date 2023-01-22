#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include<cstdio>
#include<cmath>

#include"../include/shader.hpp"

class window
{
public:
    GLFWwindow *pointer;

    window(int win_width, int win_height, const char *win_title)
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
        glfwWindowHint(GLFW_REFRESH_RATE, 60);
        pointer = glfwCreateWindow(win_width, win_height, win_title, NULL, NULL);
        if (pointer == NULL)
        {
            printf("Failed to create glfw window. Calling glfwTerminate().\n");
            glfwTerminate();
        }
        glfwMakeContextCurrent(pointer);
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK)
        {
            printf("Failed to initialize glew. Calling glfwTerminate().\n");
            glfwTerminate();
        }
        glfwSetWindowSizeLimits(pointer, 400, 400, GLFW_DONT_CARE, GLFW_DONT_CARE);
    }

    static void key_callback(GLFWwindow *pointer, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(pointer, true);
        return;
    }

    void add_key_callback()
    {
        glfwSetKeyCallback(pointer, key_callback);
        return;
    }

    static void framebuffer_size_callback(GLFWwindow *pointer, int w, int h)
    {
        if (w == 0)
            w = 1;
        if (h == 0)
            h = 1;
        
        glViewport(0,0, w,h);
        return;
    }

    void add_framebuffer_size_callback()
    {
        glfwSetFramebufferSizeCallback(pointer, framebuffer_size_callback);
        return;
    }

    ~window()
    {
        glfwTerminate();
    }

};

class gui
{
public:

    gui(GLFWwindow *pointer)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = NULL;
        (void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(pointer, true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }

    ~gui()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void new_frame()
    {
        ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
    }

    void render()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void triangle_menu(glm::vec3 &triangle_color, bool &draw_triangle)
    {
        ImGui::SetNextWindowSize(ImVec2(300.0f,200.0f), ImGuiCond_FirstUseEver); 
		ImGui::Begin("ImGui");
		ImGui::Text("Options");
		ImGui::Checkbox("Draw triangle", &draw_triangle);
		ImGui::ColorEdit3("Color", glm::value_ptr(triangle_color));
		ImGui::End();
    }
};

int main()
{
    window win(800, 800, "OpenGL");
    win.add_key_callback();
    win.add_framebuffer_size_callback();

    float verts[] = { -0.5f, -0.5f, 0.0f,
                       0.5f, -0.5f, 0.0f,
                       0.0f,  0.5f, 0.0f };

    unsigned int vao,vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0 );
    glEnableVertexAttribArray(0);

    shader shad("../shader/vertex/trans_nothing.vert",
                "../shader/fragment/monochromatic.frag");
	shad.use();

    gui ui(win.pointer);

    glm::vec3 triangle_color = glm::vec3(1.0f,0.5f,0.2f);
    bool draw_triangle = true;

    glClearColor(0.05f,0.05f,0.05f,1.0f);
    while (!glfwWindowShouldClose(win.pointer))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        shad.set_vec3_uniform("model_col",triangle_color);
        glBindVertexArray(vao);
		if (draw_triangle)
        	glDrawArrays(GL_TRIANGLES, 0, 3);

        ui.new_frame();
        ui.triangle_menu(triangle_color, draw_triangle);
        ui.render();

        glfwSwapBuffers(win.pointer);
        glfwPollEvents();
    }

    return 0;
}