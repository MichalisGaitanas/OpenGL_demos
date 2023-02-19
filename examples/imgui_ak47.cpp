#include"../imgui/imgui.h"
#include"../imgui/imgui_impl_glfw.h"
#include"../imgui/imgui_impl_opengl3.h"

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include<cstdio>

#include"../include/shader.hpp"
#include"../include/mesh.hpp"

const float pi = 3.1415926535f;

int win_width, win_height;
float aspect_ratio;

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow *window, int w, int h)
{
    glViewport(0,0, w,h);
    //inform the globals about the new size of the window, so that the we can reassign the projection matrix
    win_width  = w;
    win_height = h;
    aspect_ratio = (float)w/h;
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    //////////////////////////////////////////////////////////////////////////////////////
    win_width = win_height = 800;
    GLFWwindow *window = glfwCreateWindow(win_width,win_height, "OpenGL", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowSizeLimits(window, 300,300, GLFW_DONT_CARE,GLFW_DONT_CARE);
    glfwSetKeyCallback(window, key_callback); //register keyboard callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); //register framebuffer size callback
    //////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////
    //Inform win_width and win_height (and aspect_ratio) about the correct window size.
    //Remember, since we called glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE), the initial size will not be (win_width, win_height).
    glfwGetWindowSize(window, &win_width, &win_height);
    aspect_ratio = win_width/(float)win_height;
    //////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////
    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    //mode->width and mode->height have the correct values now
    //////////////////////////////////////////////////////////////////////////////////////

    glfwSetWindowPos( window, (mode->width - win_width)/2, (mode->height - win_height)/2 );

    //validate glew
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }

    meshvfn ak47_mesh("../obj/vfn/ak47.obj");
    shader ak47_shad("../shader/vertex/trans_mvpn.vert","../shader/fragment/dir_light_ads.frag");
    ak47_shad.use();

    glm::vec3 light_dir = glm::vec3(1.0f,0.0f,0.0f);
    glm::vec3 light_col = glm::vec3(0.0f,0.0f,1.0f);
    glm::vec3 ak47_col = glm::vec3(0.5f,0.5f,0.5f); 
    glm::vec3 cam_pos = glm::vec3(5.0f,0.0f,0.0f);
    glm::vec3 cam_aim = glm::vec3(0.0f,0.0f,0.0f);
    glm::vec3 cam_up = glm::vec3(0.0f,0.0f,1.0f);

    glm::mat4 proj;
    glm::mat4 view = glm::lookAt(cam_pos, cam_aim, cam_up);
    glm::mat4 model; float yaw = 0.0f;

    ak47_shad.set_mat4_uniform("view", view);
    ak47_shad.set_vec3_uniform("light_dir", light_dir);
    ak47_shad.set_vec3_uniform("light_col", light_col);
    ak47_shad.set_vec3_uniform("model_col", ak47_col);
    ak47_shad.set_vec3_uniform("cam_pos", cam_pos);

    IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = NULL;
	(void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    while (!glfwWindowShouldClose(window))
    {

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        proj = glm::perspective(glm::radians(45.0f), aspect_ratio, 0.01f,100.0f);
        model = glm::mat4(1.0f);
        model = glm::rotate(model, yaw, glm::vec3(0.0f,0.0f,1.0f));
        ak47_shad.set_mat4_uniform("projection", proj);
        ak47_shad.set_mat4_uniform("model", model);
        ak47_mesh.draw_triangles();

        ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(300.0f,200.0f), ImGuiCond_FirstUseEver); 
		ImGui::Begin("GUI");
        ImGui::Text("Interaction");
        ImGui::SliderFloat("yaw", &yaw, 0.0f, 2.0f*pi, "%.3f [rad]");
        ImGui::Separator();
        ImGui::Text("Info");
        ImGui::Text("[mode->width, mode->height] = [%d,%d]", mode->width,mode->height);
        ImGui::Text("[io.DisplaySize.x, io.DisplaySize.y] = [%d,%d]", (int)io.DisplaySize.x,(int)io.DisplaySize.y);
        ImGui::Text("[io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y] = [%.2f,%.2f]", io.DisplayFramebufferScale.x,io.DisplayFramebufferScale.y);
        ImGui::Text("[win_width, win_height] = [%d,%d]", win_width,win_height);
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
