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

void key_callback(GLFWwindow *window, int key, int, int action, int)
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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
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

    glm::vec3 aster_col_triangle = glm::vec3(0.5f,0.5f,0.5f);
    glm::vec3 aster_line_col = glm::vec3(0.0f,0.7f,0.8f);
    glm::vec3 aster_point_col = glm::vec3(1.0f,0.0f,0.0f);

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
        aster_shad.set_vec3_uniform("mesh_col", aster_col_triangle);
        aster.draw_triangles();
        //Play with this.
        model = glm::scale(model, glm::vec3(1.001f,1.001f,1.001f));
        aster_shad.set_mat4_uniform("model", model);
        aster_shad.set_vec3_uniform("mesh_col", aster_line_col);
        aster.draw_lines();
        model = glm::scale(model, glm::vec3(1.001f,1.001f,1.001f));
        aster_shad.set_mat4_uniform("model", model);
        aster_shad.set_vec3_uniform("mesh_col", aster_point_col);
        aster.draw_points(3.0f);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
