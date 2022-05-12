#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<cstdio>
#include"../include/shader.hpp"
#include"../include/mesh.hpp"

int win_width = 900, win_height = 900;
const char *label = "Model loading";

void process_hardware_inputs(GLFWwindow *win)
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
    //initialize glfw and some hints
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //create a window object at the center of the screen
    GLFWwindow *window = glfwCreateWindow(win_width, win_height, label, NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);
    int monitx, monity; //monitor's top left corner coordinates in the virtual space
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    glfwGetMonitorPos(monitor, &monitx, &monity);
    printf("Monitor's coordinates in virtual space : (%d,%d)\n",monitx,monity);
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    glfwSetWindowPos( window, (monitx + mode->width - win_width)/2, (monity + mode->height - win_height)/2 );
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); //set framebuffer size callback function

    //validate glew
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }

    mesh suzanne("../obj/vert_face_snorm/suzanne.obj", 1,1,1);
    mesh lamp("../obj/vert_face_fnorm/cube2x2x2.obj", 1,1,0);

    //create shaders
    shader suzanne_shad("../shader/trans_mvpn.vert","../shader/point_light_ads.frag");
    shader lamp_shad("../shader/trans_mvp.vert","../shader/monochromatic.frag");

    glm::vec3 light_pos = glm::vec3(0.0f,-2.5f,1.0f); //light position in world coordinates
    glm::vec3 light_col = glm::vec3(1.0f,1.0f,1.0f); //lighting calculations color
    glm::vec3 suzanne_col = glm::vec3(0.5f,0.5f,0.5f); //suzanne color
    glm::vec3 lamp_col = glm::vec3(1.0f,1.0f,1.0f); //lamp color
    glm::vec3 cam_pos = glm::vec3(-5.0f,-5.0f,1.0f); //camera position in world coordinates
    glm::vec3 cam_aim = glm::vec3(0.0f,0.0f,0.0f); //camera aim direction
    glm::vec3 cam_up = glm::vec3(0.0f,0.0f,1.0f); //camera up direction

    glm::mat4 projection = glm::perspective(glm::radians(45.0f),(float)win_width/(float)win_height, 0.01f,100.0f);
    glm::mat4 view = glm::lookAt(cam_pos, cam_aim, cam_up);
    glm::mat4 model = glm::mat4(1.0f);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    while (!glfwWindowShouldClose(window)) //render loop
    {
        process_hardware_inputs(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        model = glm::mat4(1.0f);
        model = glm::rotate(model, (0.5f*(float)glfwGetTime()), glm::vec3(1.0f,1.0f,1.0f));
        suzanne_shad.use();
        suzanne_shad.set_mat4_uniform("projection", projection);
        suzanne_shad.set_mat4_uniform("view", view);
        suzanne_shad.set_mat4_uniform("model", model);
        suzanne_shad.set_vec3_uniform("light_pos", light_pos);
        suzanne_shad.set_vec3_uniform("cam_pos", cam_pos);
        suzanne_shad.set_vec3_uniform("light_col", light_col);
        suzanne_shad.set_vec3_uniform("model_col", suzanne_col);
        suzanne.draw_triangles();

        lamp_shad.use();
        model = glm::mat4(1.0f);
        model = glm::translate(model, light_pos);
        model = glm::scale(model, glm::vec3(0.25f,0.25f,0.25f));
        lamp_shad.set_mat4_uniform("projection", projection);
        lamp_shad.set_mat4_uniform("view", view);
        lamp_shad.set_mat4_uniform("model", model);
        lamp_shad.set_vec3_uniform("model_col", lamp_col);
        lamp.draw_triangles();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
