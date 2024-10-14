#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<cstdio>

#include"../include/shader.hpp"

const int win_width = 800, win_height = 700;
const char *win_label = "Triangle shader class";

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
    printf("[w,h] = [%d,%d]\n",w,h);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); //Anti aliasing.
    
    GLFWwindow *win = glfwCreateWindow(win_width, win_height, win_label, NULL, NULL);
    if (win == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(win);
    glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);
    glfwSetKeyCallback(win, key_callback);
    
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }
    
    //Define the coordinates of the vertices of the triangle to be rendered.
    float verts[] = { -0.5f, -0.5f, 0.0f,
                       0.5f, -0.5f, 0.0f,
                       0.0f,  0.5f, 0.0f };
    
    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0 );
    glEnableVertexAttribArray(0);
    
    //Create a shader object.
    shader shad("../shaders/vertex/trans_nothing.vert","../shaders/fragment/monochromatic.frag");
    glm::vec3 triangle_col = glm::vec3(1.0f,0.5f,0.2f);
    
    glClearColor(0.1f,0.5f,0.2f,1.0f);
    while (!glfwWindowShouldClose(win))
    { 
        glClear(GL_COLOR_BUFFER_BIT);

        shad.use(); //Use the "shad" shader object ( same as glUseProgram(shad_ID) )
        shad.set_vec3_uniform("mesh_col",triangle_col);// Inform the shader about this variable.
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        glfwSwapBuffers(win);
        glfwPollEvents();
    }
    
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    
    //Shader resources are deleted in the class destructror.
    
    glfwTerminate();
    return 0;
}
