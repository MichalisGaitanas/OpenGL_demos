#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<cstdio>
#include<cmath>

int win_width = 1200, win_height = 750;

//Vertex shader source code.
const char *vsource = "#version 450 core\n"
                      "layout (location = 0) in vec3 pos;\n"
                      "void main()\n"
                      "{\n"
                      "   gl_Position = vec4(pos, 1.0f);\n"
                      "}";

//Fragment shader source code.
const char *fsource = "#version 450 core\n"
                      "out vec4 frag_color;\n"
                      "uniform float intensity;\n"
                      "void main()\n"
                      "{\n"
                      "   frag_color = intensity*vec4(1.0f,0.0f,0.0f,1.0f);\n"
                      "}";


void key_callback(GLFWwindow *win, int key, int, int action, int)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
    {
        glfwSetWindowShouldClose(win, true);
    }
}

void framebuffer_size_callback(GLFWwindow *, int w, int h)
{
    win_width = w;
    win_height = h;
    glViewport(0,0,w,h);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); //Anti aliasing.
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE); //Windowed fullscreen.

    GLFWwindow *win = glfwCreateWindow(win_width, win_height, "Triangle dynamic color", NULL, NULL);
    if (win == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(win);

    //Set the callbacks.
    glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);
    glfwSetKeyCallback(win, key_callback);

    glfwSetWindowSizeLimits(win, 400, 400, GLFW_DONT_CARE, GLFW_DONT_CARE);
    
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }
    
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

    glUseProgram(shader_prog);
    
    glClearColor(0.1f,0.1f,0.1f,1.0f);
    while (!glfwWindowShouldClose(win))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        
        float t = (float)glfwGetTime(); //Elapsed time in seconds since glfwInit() was called.
        printf("\r%d [sec]", (int)t); fflush(stdout); //Print time in the terminal.
        float intensity = (float)fabs(cos(t)); //Let the intensity of the triangle be dynamic (changing) based on f(t) = |sin(t)|.
        int intensity_location = glGetUniformLocation(shader_prog, "intensity"); //Intensity variable location so that the shader knows about it to update it.
        glUniform1f(intensity_location, intensity);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shader_prog);
    glDeleteShader(vshader);
    glDeleteShader(fshader);
    
    glfwTerminate();
    return 0;
}
