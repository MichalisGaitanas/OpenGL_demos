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

int win_width = 1000, win_height = 800;

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow *win, int w, int h)
{
    win_width = w;
    win_height = h;
    glViewport(0,0,w,h);
}

//Center the window.
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
    
    GLFWwindow *window = glfwCreateWindow(win_width, win_height, "Mesh control via GUI", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);
    glfw_center_window(window);
    
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }
    
    //Define the coordinates of the vertices of the mesh to be rendered.
    float verts[] = { -0.5f, -0.5f, 0.0f,
                       0.5f, -0.5f, 0.0f,
                       0.0f,  0.5f, 0.0f };
    
    //Setup memory passes in gpu.
    unsigned int vao,vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0 );
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    
    //Setup shader.
    shader shad("../shaders/vertex/trans_nothing.vert","../shaders/fragment/monochromatic.frag");
	shad.use();

    //Setup ImGui.
	IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = NULL;
    io.Fonts->AddFontFromFileTTF("../fonts/Arial.ttf", 15.0f);
    (void)io;
    ImGui::StyleColorsLight();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGuiStyle &imstyle = ImGui::GetStyle();
    imstyle.WindowMinSize = ImVec2(100.0f,100.0f);
    imstyle.FrameRounding = 5.0f;
    imstyle.WindowRounding = 5.0f;

	//We will expose the following variables to gui window.
	glm::vec3 mesh_color = glm::vec3(1.0f,0.5f,0.2f); //Mesh color.
    glm::vec3 clear_color = glm::vec3(0.0f,0.5f,0.0f); //Clear color.
	bool draw_mesh = true; //Visibility.
    bool popen = true;

	while (!glfwWindowShouldClose(window))
	{
        glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
		//Only draw the triangle if the imgui checkbox is checked.
		if (draw_mesh)
        {
            shad.set_vec3_uniform("mesh_col", mesh_color);
            glBindVertexArray(vao);
        	glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);
        }

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(300.0f,200.0f), ImGuiCond_FirstUseEver); 
		ImGui::Begin("GUI controls", &popen);
        if (!popen)
            glfwSetWindowShouldClose(window, true);

		ImGui::Checkbox("Draw mesh", &draw_mesh);
		ImGui::ColorEdit3("Mesh color", glm::value_ptr(mesh_color));
        ImGui::ColorEdit3("Clear color", glm::value_ptr(clear_color));
		
        ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);

	glfwTerminate();

	return 0;
}