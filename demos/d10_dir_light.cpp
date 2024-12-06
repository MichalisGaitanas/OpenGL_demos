#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include<cstdio>
#include<cmath>

#include"../include/shader.h"
#include"../include/mesh.h"

int win_width = 800, win_height = 800;

//When a keyboard key is pressed :
void key_callback(GLFWwindow *window, int key, int, int action, int)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

//When the framebuffer is resized :
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

    GLFWwindow *window = glfwCreateWindow(win_width, win_height, "Directional light", NULL, NULL);
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
        printf("Failed to initialize glew. Exiting...");
        return 0;
    }

    meshvfn sphere("../obj/vfn/uv_sphere_rad1_40x30.obj");
    shader shad_sphere("../shaders/vertex/trans_mvpn.vert", "../shaders/fragment/dir_light_ad.frag");
    shad_sphere.use();

    glm::vec3 light_dir; //Light direction in world coordinates.
    glm::vec3 light_col = glm::vec3(1.0f,1.0f,1.0f); //Light color.
    glm::vec3 sphere_col = glm::vec3(1.0f,0.0f,0.0f); //Sphere color.
    glm::vec3 cam_pos = glm::vec3(0.0f,-7.0f,2.0f); //Camera position in world coordinates.
    glm::vec3 cam_aim = glm::vec3(0.0f,0.0f,0.0f); //Camera aim (eye) direction in world coordinates.
    glm::vec3 cam_up = glm::vec3(0.0f,0.0f,1.0f); //Camera up direction in world coordinates.

    //Projection, view and model matrices definition.
    glm::mat4 projection; //This will change at each frame due to win_width and win_height.
    glm::mat4 view = glm::lookAt(cam_pos, cam_aim, cam_up); //This will be constant.
    glm::mat4 model = glm::mat4(1.0f);

    shad_sphere.set_mat4_uniform("view", view);
    shad_sphere.set_mat4_uniform("model", model);
    shad_sphere.set_vec3_uniform("light_col", light_col);
    shad_sphere.set_vec3_uniform("mesh_col", sphere_col);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clear color buffer and z-buffer.

        projection = glm::perspective(glm::radians(45.0f), (float)win_width/win_height, 0.01f,100.0f);
        shad_sphere.set_mat4_uniform("projection", projection);

        light_dir = glm::vec3(cos(glfwGetTime()), sin(glfwGetTime()), sin(glfwGetTime())); //Revolving light.
        shad_sphere.set_vec3_uniform("light_dir", light_dir);
        sphere.draw_triangles();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
