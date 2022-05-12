#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<cstdio>
#include<cmath>

const int win_width = 900, win_height = 750;
const char *win_label = "Triangle dynamic intensity";

//vertex shader source code
const char *vsource = "#version 330 core\n"
                      "layout (location = 0) in vec3 pos;\n"
                      "void main()\n"
                      "{\n"
                      "   gl_Position = vec4(pos, 1.0f);\n"
                      "}";

//fragment shader source code
const char *fsource = "#version 330 core\n"
                      "out vec4 frag_color;\n"
                      "uniform float intensity;\n"
                      "void main()\n"
                      "{\n"
                      "   frag_color = intensity*vec4(1.0f,0.0f,0.0f,1.0f);\n"
                      "}";

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
    
    unsigned int vao,vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0 );
    glEnableVertexAttribArray(0);
    
    //vertex shader stuff
    unsigned int vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, 1, &vsource, NULL);
    glCompileShader(vshader);
    int success;
    char infolog[512];
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vshader, 512, NULL, infolog);
        printf("Error while compiling 'vertex' shader.\n");
        printf("%s\n",infolog);
    }
    
    //fragment shader stuff
    unsigned int fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, 1, &fsource, NULL);
    glCompileShader(fshader);
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fshader, 512, NULL, infolog);
        printf("Error while compiling 'fragment' shader.\n");
        printf("%s\n",infolog);
    }
    
    //linking stuff
    unsigned int shader_prog = glCreateProgram();
    glAttachShader(shader_prog, vshader);
    glAttachShader(shader_prog, fshader);
    glLinkProgram(shader_prog);
    glGetProgramiv(shader_prog, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shader_prog, 512, NULL, infolog);
        printf("Error while linking shader program.\n");
        printf("%s\n",infolog);
    }
    
    glClearColor(0.1f,0.1f,0.1f,1.0f); //background color
    while (!glfwWindowShouldClose(win)) //game loop
    {
        process_hardware_inputs(win);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shader_prog);
            float t = glfwGetTime(); //elapsed time in seconds since glfwInit() was called
            printf("\r %d [sec]", (int)t); fflush(stdout); //print time in the terminal
            float intensity = (float)fabs(cos(t)); //let the intensity of the triangle be dynamic (changing) based on f(t) = |sin(t)|
            int intensity_location = glGetUniformLocation(shader_prog, "intensity"); //intensity variable location so that the shaders know about it
            glUniform1f(intensity_location, intensity);
            glBindVertexArray(vao);
            glDrawArrays(GL_TRIANGLES, 0, 3);
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
