#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<cstdio>

const int x_top_left = 100, y_top_left = 100; //top left window coordinates
const int win_width = 800, win_height = 700;
const char *win_label = "First triangle";

//vertex shader source code
const char *vsource = "#version 330 core\n"
                      "layout (location = 0) in vec3 pos;\n"
                      "void main()\n"
                      "{\n"
                      "   gl_Position = vec4(pos.x, pos.y, pos.z, 1.0f);\n"
                      "}";

//fragment shader source code
const char *fsource = "#version 330 core\n"
                      "out vec4 frag_color;\n"
                      "void main()\n"
                      "{\n"
                      "   frag_color = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
                      "}";

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
    glfwSetWindowPos(win, x_top_left, y_top_left); //set initial position of the window
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

    //tell the computer how to store and read the previously defined geometry - data in order to be able to process/render it
    unsigned int vao,vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0 );
    glEnableVertexAttribArray(0);

    //read and compile vertex shader
    unsigned int vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, 1, &vsource, NULL);
    glCompileShader(vshader);
    int success;
    char infolog[512];
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
    if (!success) //report compilation error
    {
        glGetShaderInfoLog(vshader, 512, NULL, infolog);
        printf("Error while compiling 'vertex' shader.\n");
        printf("%s\n",infolog);
    }

    //read and compile fragment shader
    unsigned int fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, 1, &fsource, NULL);
    glCompileShader(fshader);
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
    if (!success) //report compilation error
    {
        glGetShaderInfoLog(fshader, 512, NULL, infolog);
        printf("Error while compiling 'fragment' shader.\n");
        printf("%s\n",infolog);
    }

    //create and link a "chained" shader program(s)
    unsigned int shader_prog = glCreateProgram();
    glAttachShader(shader_prog, vshader);
    glAttachShader(shader_prog, fshader);
    glLinkProgram(shader_prog);
    glGetProgramiv(shader_prog, GL_LINK_STATUS, &success);
    if (!success) //report linkage error
    {
        glGetProgramInfoLog(shader_prog, 512, NULL, infolog);
        printf("Error while linking shader program.\n");
        printf("%s\n",infolog);
    }

    glClearColor(0.1f,0.5f,0.2f,1.0f); //background color
    while (!glfwWindowShouldClose(win)) //game loop
    {
        raw_hardware_input(win);

        glClear(GL_COLOR_BUFFER_BIT); //constantly refresh the backgound color in accordance with the glClearColor()

        glUseProgram(shader_prog);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3); //actually plot

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    //free resources
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shader_prog);

    glDeleteShader(vshader);
    glDeleteShader(fshader);

    glfwTerminate();
    return 0;
}
