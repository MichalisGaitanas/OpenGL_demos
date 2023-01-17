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

void raw_hardware_input(GLFWwindow *win)
{
    if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(win, true);
    }
}

void framebuffer_size_callback(GLFWwindow *win, int w, int h)
{
    glViewport(0,0,w,h);
}

int main()
{
	//initialize glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    //create a window object
    const int win_width = 800, win_height = 700;
    GLFWwindow *win = glfwCreateWindow(win_width, win_height, "Triangle", NULL, NULL);
    if (win == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(win);
    glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);
    
    //validate glew
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }
    
    //define the coordinates of the vertices of the triangle to be rendered
    float verts[] = { -0.5f, -0.5f, 0.0f,
                       0.5f, -0.5f, 0.0f,
                       0.0f,  0.5f, 0.0f };
    
    //tell the gpu how it should read the previously defined geometry - data in order to be able to process it
    unsigned int vao,vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0 );
    glEnableVertexAttribArray(0);
    
    //creating a shader object
    shader shad("../shader/vertex/trans_nothing.vert","../shader/fragment/monochromatic.frag");
	shad.use(); //activate the "shad" shader object ( same as glUseProgram(shad_ID) )

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = NULL;
	(void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(win, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	//variable to be changed in the imgui window
	glm::vec3 triangle_col = glm::vec3(1.0f,0.5f,0.2f);
	bool draw_triangle = true;

	glClearColor(0.1f,0.5f,0.2f,1.0f); //background color
	while (!glfwWindowShouldClose(win))
	{
		raw_hardware_input(win);
        
        glClear(GL_COLOR_BUFFER_BIT);
        
        shad.set_vec3_uniform("model_col",triangle_col);
        glBindVertexArray(vao);
		//only draw the triangle if the imgui checkbox is checked
		if (draw_triangle)
        	glDrawArrays(GL_TRIANGLES, 0, 3);

		//tell OpenGL a new frame is about to begin
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(300.0f,300.0f), ImGuiCond_FirstUseEver); 
		ImGui::Begin("Triangle");

		ImGui::Text("Navigate");
		ImGui::Checkbox("Draw triangle", &draw_triangle);
		ImGui::ColorEdit3("Color", glm::value_ptr(triangle_col)); //color editor that appears in the window
		ImGui::End();

		//Renders the imgui elements
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(win);
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