#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<cstdio>
#include"../include/shader.hpp"

int win_width = 900, win_height = 900;
const char *label = "Point light calculations";

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

    //vertices and normals of the cube
    float geometry[] = {
                            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
                             0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
                             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
                             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
                            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
                            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

                            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
                             0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
                             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
                             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
                            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
                            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

                            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
                            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
                            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
                            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
                            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
                            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

                             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
                             0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
                             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
                             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
                             0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
                             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

                            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
                             0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
                             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
                             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
                            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
                            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

                            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
                             0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
                             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
                             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
                            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
                            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };

    //create cube vao and vbo stuff
    unsigned int cube_vao, cube_vbo;
    glGenVertexArrays(1, &cube_vao);
    glBindVertexArray(cube_vao);
    glGenBuffers(1, &cube_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(geometry), geometry, GL_STATIC_DRAW);
    //vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //normal attributes
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    //create lamp (which is also a cube) vao and vbo stuff
    unsigned int lamp_vao, lamp_vbo;
    glGenVertexArrays(1, &lamp_vao);
    glBindVertexArray(lamp_vao);
    glGenBuffers(1, &lamp_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, lamp_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(geometry), geometry, GL_STATIC_DRAW);
    //only vertex attributes are handled here because we want a single color for the lamp, thus no normals are needed
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //create shaders
    shader cube_shad("../shaders/vertex/trans_mvpn.vert","../shaders/fragment/point_light_ads.frag");
    shader lamp_shad("../shaders/vertex/trans_mvp.vert","../shaders/fragment/monochromatic.frag");

    glEnable(GL_DEPTH_TEST); //enable depth - testing
    glClearColor(0.0f,0.0f,0.0f,1.0f); //background color
    while (!glfwWindowShouldClose(window)) //game loop
    {
        process_hardware_inputs(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::vec3 light_pos = glm::vec3(0.0f,-2.5f,1.0f); //light position in world coordinates
        glm::vec3 light_col = glm::vec3(1.0f,1.0f,1.0f); //lighting calculations color
        glm::vec3 cube_col = glm::vec3(0.8f,0.0f,0.0f); //model color
        glm::vec3 lamp_col = glm::vec3(1.0f,1.0f,1.0f); //lamp color
        glm::vec3 cam_pos = glm::vec3(-5.0f,-5.0f,1.0f); //camera position in world coordinates

        //win_width, win_height are never updated because there's no corresponding "update code" in framebuffer_size_callback()
        glm::mat4 projection = glm::perspective(glm::radians(45.0f),(float)win_width/(float)win_height, 0.01f,100.0f);
        glm::mat4 view = glm::lookAt(cam_pos, glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,0.0f,1.0f));
        glm::mat4 model = glm::mat4(1.0f);

        cube_shad.use();
        model = glm::rotate(model, (0.5f*(float)glfwGetTime()), glm::vec3(1.0f,1.0f,1.0f));
        cube_shad.set_mat4_uniform("projection", projection);
        cube_shad.set_mat4_uniform("view", view);
        cube_shad.set_mat4_uniform("model", model);
        cube_shad.set_vec3_uniform("light_pos", light_pos);
        cube_shad.set_vec3_uniform("cam_pos", cam_pos);
        cube_shad.set_vec3_uniform("light_col", light_col);
        cube_shad.set_vec3_uniform("model_col", cube_col);
        glBindVertexArray(cube_vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        lamp_shad.use();
        model = glm::mat4(1.0f);
        model = glm::translate(model, light_pos);
        model = glm::scale(model, glm::vec3(0.5f,0.5f,0.5f));
        lamp_shad.set_mat4_uniform("projection", projection);
        lamp_shad.set_mat4_uniform("view", view);
        lamp_shad.set_mat4_uniform("model", model);
        lamp_shad.set_vec3_uniform("model_col", lamp_col);
        glBindVertexArray(lamp_vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &cube_vao);
    glDeleteBuffers(1, &cube_vbo);
    glDeleteVertexArrays(1, &lamp_vao);
    glDeleteBuffers(1, &lamp_vbo);

    glfwTerminate();
    return 0;
}
