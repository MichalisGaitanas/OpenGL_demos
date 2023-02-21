#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<cstdio>
#include"../include/shader.hpp" //inlcude the custom shader header

const int win_width = 800, win_height = 700;
const char *win_label = "Triangle shader class";

void raw_hardware_input(GLFWwindow *win)
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
    //initialize glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    //create a window object
    GLFWwindow *win = glfwCreateWindow(win_width, win_height, win_label, NULL, NULL);
    if (win == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(win);
    glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);
    
    //validate glew
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }
    
    //define the coordinates of the vertices of the triangle to be rendered
    float verts[] = { -0.5f, -0.5f, 0.0f,
                       0.5f, -0.5f, 0.0f,
                       0.0f,  0.5f, 0.0f };
    
    //tell the gpu how it should read the previously defined geometry - data in order to be able to process it
    unsigned int vao,vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0 );
    glEnableVertexAttribArray(0);
    
    //creating a shader object
    shader shad("../shaders/vertex/trans_nothing.vert","../shaders/fragment/monochromatic.frag");
    glm::vec3 triangle_col = glm::vec3(1.0f,0.5f,0.2f);
    
    glClearColor(0.1f,0.5f,0.2f,1.0f); //background color
    while (!glfwWindowShouldClose(win))
    {
        raw_hardware_input(win);
        
        glClear(GL_COLOR_BUFFER_BIT);
        shad.use(); //use the "shad" shader object ( same as glUseProgram(shad_ID) )
        shad.set_vec3_uniform("model_col",triangle_col);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        glfwSwapBuffers(win);
        glfwPollEvents();
    }
    
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    
    //shader resources are deleted in the class
    
    glfwTerminate();
    return 0;
}
