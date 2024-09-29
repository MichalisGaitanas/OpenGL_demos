#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include<cstdio>
#include<cmath>

#include"../include/shader.hpp"

void key_callback(GLFWwindow *win, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
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
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); //Anti aliasing.


    int width = 800, height = 800;
    GLFWwindow *win = glfwCreateWindow(width, height, "Rotating square", NULL, NULL);
    if (win == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(win);
    int monitx, monity;
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    glfwGetMonitorPos(monitor, &monitx, &monity);
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    glfwSetWindowPos( win, (monitx + mode->width - width)/2, (monity + mode->height - height)/2 );
    glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);
    glfwSetKeyCallback(win, key_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }

    //Define the coordinates of the vertices of the square to be rendered.
    float verts[] = { -0.5f, -0.5f, 0.0f,
                       0.5f, -0.5f, 0.0f,
                       0.5f,  0.5f, 0.0f, //First triangle.

                      -0.5f, -0.5f, 0.0f,
                       0.5f,  0.5f, 0.0f,
                      -0.5f,  0.5f, 0.0f }; //Second triangle.

    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0 );
    glEnableVertexAttribArray(0);

    shader shad("../shaders/vertex/trans_m.vert","../shaders/fragment/monochromatic.frag");
    shad.use(); //Activate the "shad" shader object.
    glm::vec3 mesh_col = glm::vec3(1.0f,0.1f,0.1f); //Set the color of the mesh.
    shad.set_vec3_uniform("mesh_col", mesh_col); //Inform the shader about the color of the mesh via the uniform variable 'mesh_col'.

    glm::mat4 model; //define the model matrix variable.

    glClearColor(1.0f,1.0f,1.0f,1.0f); //Background color.
    while (!glfwWindowShouldClose(win)) //Rendering loop.
    {

        glClear(GL_COLOR_BUFFER_BIT);

        //Set all vertices to have identity matrix, i.e. with respect to the world coordinates, they are at the positions set by verts[].
        model = glm::mat4(1.0f);

        //Rotate all vertices around the z-axis. This happens by multiplying (from the left) the model matrix (identity) with a rotation matrix and then with the vertex coordinates.
        model = glm::rotate(model, (float)sin((float)glfwGetTime()), glm::vec3(0.0f,0.0f,1.0f));
        
        //Inform the shader about the matrix of the mesh via the uniform variable 'model'.
        shad.set_mat4_uniform("model", model);
        
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);

    glfwTerminate();
    return 0;
}
