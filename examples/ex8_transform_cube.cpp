#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include<cstdio>
#include<cmath>

#include"../include/shader.hpp"

const double pi = 3.141592653589793238462;

int win_width = 800, win_height = 800;

bool motion = false;

//When a keyboard key is pressed :
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        motion = !motion;
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

    GLFWwindow *window = glfwCreateWindow(win_width, win_height, "3D or 2D?", NULL, NULL);
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

    //Define the coordinates of the vertices of the cube to be rendered.
    float verts[] = { -0.5f, -0.5f, -0.5f,
                       0.5f, -0.5f, -0.5f,
                       0.5f,  0.5f, -0.5f,
                       0.5f,  0.5f, -0.5f,
                      -0.5f,  0.5f, -0.5f,
                      -0.5f, -0.5f, -0.5f, //down face

                      -0.5f, -0.5f,  0.5f,
                       0.5f, -0.5f,  0.5f,
                       0.5f,  0.5f,  0.5f,
                       0.5f,  0.5f,  0.5f,
                      -0.5f,  0.5f,  0.5f,
                      -0.5f, -0.5f,  0.5f, //up face

                      -0.5f,  0.5f,  0.5f,
                      -0.5f,  0.5f, -0.5f,
                      -0.5f, -0.5f, -0.5f,
                      -0.5f, -0.5f, -0.5f,
                      -0.5f, -0.5f,  0.5f,
                      -0.5f,  0.5f,  0.5f, //left face

                       0.5f,  0.5f,  0.5f,
                       0.5f,  0.5f, -0.5f,
                       0.5f, -0.5f, -0.5f,
                       0.5f, -0.5f, -0.5f,
                       0.5f, -0.5f,  0.5f,
                       0.5f,  0.5f,  0.5f, //right face

                      -0.5f, -0.5f, -0.5f,
                       0.5f, -0.5f, -0.5f,
                       0.5f, -0.5f,  0.5f,
                       0.5f, -0.5f,  0.5f,
                      -0.5f, -0.5f,  0.5f,
                      -0.5f, -0.5f, -0.5f, //back face

                      -0.5f,  0.5f, -0.5f,
                       0.5f,  0.5f, -0.5f,
                       0.5f,  0.5f,  0.5f,
                       0.5f,  0.5f,  0.5f,
                      -0.5f,  0.5f,  0.5f,
                      -0.5f,  0.5f, -0.5f  }; //front face

    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0 );
    glEnableVertexAttribArray(0);

    shader shad("../shaders/vertex/trans_mp.vert","../shaders/fragment/monochromatic.frag");
    shad.use();

    glm::vec3 mesh_col = glm::vec3(1.0f,0.1f,0.1f);
    shad.set_vec3_uniform("mesh_col",mesh_col);

    //Enable depth testing. This is basically an automated algorithm by OpenGL to render only the triangles that are really in front in the 3D space and
    //discard the rendering commands of the triangles that are 'backer' with respect to the truly front ones.
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.0f,0.0f,0.0f,1.0f);

    float tnow = 0.0f;

    while (!glfwWindowShouldClose(window)) //Game loop.
    {
        //printf("tnow = %f\n", tnow);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)win_width/win_height, 0.01f,100.0f);
        shad.set_mat4_uniform("projection", projection);

        //Render 1st cube.
        glm::mat4 model = glm::mat4(1.0f);
        if (motion)
        {
            tnow = (float)glfwGetTime();
            model = glm::translate(model, glm::vec3(sin(tnow),0.0f,-5.0f));
            model = glm::rotate(model, tnow, glm::vec3(1.0f,1.0f,1.0f));
        }
        else //Freeze.
        {
            glfwSetTime((double)tnow);
            model = glm::translate(model, glm::vec3(sin(tnow),0.0f,-5.0f));
            model = glm::rotate(model, tnow, glm::vec3(1.0f,1.0f,1.0f));
        }
        shad.set_mat4_uniform("model", model);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        //Render 2nd cube.
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.0f,2.0f,-7.0f));
        model = glm::rotate(model, (float)pi/3.0f, glm::vec3(1.0f,1.0f,1.0f));
        shad.set_mat4_uniform("model", model);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);

    glfwTerminate();
    return 0;
}
