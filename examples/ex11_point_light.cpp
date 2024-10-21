#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include<cstdio>

#include"../include/shader.hpp"
#include"../include/mesh.hpp"

int win_width = 900, win_height = 900;

//When a keyboard key is pressed :
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
        glfwSetWindowShouldClose(window, true);
}

//When the framebuffer is resized :
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

    GLFWwindow *window = glfwCreateWindow(win_width, win_height, "Directional lighting", NULL, NULL);
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

    meshvf lamp_mesh("../obj/vf/uv_sphere_rad1_40x40.obj");
    shader lamp_shad("../shaders/vertex/trans_mvp.vert", "../shaders/fragment/monochromatic.frag");

    meshvfn cube_mesh("../obj/vfn/cube2x2x2.obj");
    shader cube_shad("../shaders/vertex/trans_mvpn.vert", "../shaders/fragment/point_light_ads.frag");
 
    glm::vec3 light_col = glm::vec3(1.0f,1.0f,1.0f);
    glm::vec3 lamp_col = glm::vec3(1.0f,1.0f,1.0f);
    glm::vec3 lamp_pos = glm::vec3(3.0f,-3.0f,2.0f);
    glm::vec3 cube_col = glm::vec3(0.8f,0.4f,0.0f);
    glm::vec3 cam_pos = glm::vec3(7.0f,3.0f,4.0f);
    glm::vec3 cam_aim = glm::vec3(0.0f,0.0f,0.0f);
    glm::vec3 cam_up = glm::vec3(0.0f,0.0f,1.0f);
    
    glm::mat4 projection, view, model;
    view = glm::lookAt(cam_pos, cam_aim, cam_up);

    //Pre-pass to the shaders all the variables that will not change in the while loop.

    lamp_shad.use();
    lamp_shad.set_vec3_uniform("mesh_col", lamp_col);
    lamp_shad.set_mat4_uniform("view", view);

    cube_shad.use();
    cube_shad.set_vec3_uniform("mesh_col", cube_col);
    cube_shad.set_vec3_uniform("light_col", light_col);
    cube_shad.set_vec3_uniform("light_pos", lamp_pos);
    cube_shad.set_vec3_uniform("cam_pos", cam_pos);
    cube_shad.set_mat4_uniform("view", view);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        projection = glm::perspective(glm::radians(45.0f),(float)win_width/(float)win_height, 0.01f,100.0f);

        //Cube :
        cube_shad.use();
        model = glm::rotate(glm::mat4(1.0f), (0.3f*(float)glfwGetTime()), glm::vec3(1.0f,1.0f,1.0f));
        cube_shad.set_mat4_uniform("projection", projection);
        cube_shad.set_mat4_uniform("model", model);
        cube_mesh.draw_triangles();

        //Lamp :
        lamp_shad.use();
        model = glm::translate(glm::mat4(1.0f), lamp_pos);
        model = glm::scale(model, glm::vec3(0.5f,0.5f,0.5f)); //Scale down uniformy the size of the lamp mesh.
        lamp_shad.set_mat4_uniform("projection", projection);
        lamp_shad.set_mat4_uniform("model", model);
        lamp_mesh.draw_triangles();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
