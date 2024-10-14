#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<cstdio>

#include"../include/shader.hpp"
#include"../include/mesh.hpp"

int win_width = 1000, win_height = 800;
const char *win_label = "Mesh loading";

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
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

    GLFWwindow *window = glfwCreateWindow(win_width, win_height, win_label, NULL, NULL);
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

    meshvf aster("../obj/vf/didymain2019.obj");
    shader aster_shad("../shaders/vertex/trans_mvp.vert","../shaders/fragment/monochromatic.frag");
    aster_shad.use();

    glm::vec3 aster_col = glm::vec3(0.5f,0.5f,0.5f);
    aster_shad.set_vec3_uniform("mesh_col", aster_col);

    //Virtual camera's settings.
    glm::vec3 cam_pos = glm::vec3(-1.0f,-1.0f,1.0f);
    glm::vec3 cam_aim = glm::vec3(0.0f,0.0f,0.0f);
    glm::vec3 cam_up = glm::vec3(0.0f,0.0f,1.0f);

    glm::mat4 projection, view, model;
    view = glm::lookAt(cam_pos, cam_aim, cam_up);
    aster_shad.set_mat4_uniform("view", view);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    while (!glfwWindowShouldClose(window)) //Render loop.
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        projection = glm::perspective(glm::radians(45.0f), (float)win_width/(float)win_height, 0.01f,100.0f);
        aster_shad.set_mat4_uniform("projection", projection);

        model = glm::rotate(glm::mat4(1.0f), (0.5f*(float)glfwGetTime()), glm::vec3(1.0f,1.0f,1.0f));
        aster_shad.set_mat4_uniform("model", model);
        
        aster.draw_triangles();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
