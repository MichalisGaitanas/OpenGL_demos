#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<cstdio>

//Vertex shader source code.
const char *vsource = "#version 450 core\n"
                      "layout (location = 0) in vec3 pos;\n"
                      "void main()\n"
                      "{\n"
                      "   gl_Position = vec4(pos.x, pos.y, pos.z, 1.0f);\n"
                      "}";

//Fragment shader source code.
const char *fsource = "#version 450 core\n"
                      "out vec4 frag_color;\n"
                      "void main()\n"
                      "{\n"
                      "   frag_color = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
                      "}";

void key_callback(GLFWwindow *win, int key, int /*scancode*/, int action, int /*mods*/)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
    {
        glfwSetWindowShouldClose(win, true);
    }
}

//When the window size changes, the framebuffer size also changes. When this happens, do the following :
void framebuffer_size_callback(GLFWwindow */*win*/, int w, int h)
{
    glViewport(0,0,w,h); //Set the viewport to cover the new window dimensions (entire window).
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    const int x_top_left = 100, y_top_left = 100;
    const int win_width = 800, win_height = 700;
    const char *win_label = "First triangle";
    GLFWwindow *win = glfwCreateWindow(win_width, win_height, win_label, NULL, NULL);
    if (win == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(win);
    glfwSetWindowPos(win, x_top_left, y_top_left);
    glfwSetKeyCallback(win, key_callback);
    glfwSetFramebufferSizeCallback(win, framebuffer_size_callback); //Register the framebuffer size callback.

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }

    //Define the coordinates of the vertices of the triangle (mesh) to be rendered.
    float verts[] = { -0.5f, -0.5f, 0.0f,
                       0.5f, -0.5f, 0.0f,
                       0.0f,  0.5f, 0.0f };

    //Tell the gpu how to store the previously defined geometry data and how to access it when the
    //time comes in order to be able to process/render it.
    unsigned int vao,vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); //Upload vertex data to GPU.
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0 );
    glEnableVertexAttribArray(0);

    //Read and compile vertex shader.
    unsigned int vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, 1, &vsource, NULL);
    glCompileShader(vshader);
    int success;
    char infolog[512];
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
    if (!success) //Report compilation error.
    {
        glGetShaderInfoLog(vshader, 512, NULL, infolog);
        printf("Error while compiling 'vertex' shader.\n");
        printf("%s\n",infolog);
    }

    //Read and compile fragment shader.
    unsigned int fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, 1, &fsource, NULL);
    glCompileShader(fshader);
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
    if (!success) //Report compilation error.
    {
        glGetShaderInfoLog(fshader, 512, NULL, infolog);
        printf("Error while compiling 'fragment' shader.\n");
        printf("%s\n",infolog);
    }

    //Create and link a "chained" shader program.
    unsigned int shader_prog = glCreateProgram();
    glAttachShader(shader_prog, vshader);
    glAttachShader(shader_prog, fshader);
    glLinkProgram(shader_prog);
    glGetProgramiv(shader_prog, GL_LINK_STATUS, &success);
    if (!success) //Report linking error.
    {
        glGetProgramInfoLog(shader_prog, 512, NULL, infolog);
        printf("Error while linking shader program.\n");
        printf("%s\n",infolog);
    }

    glClearColor(0.1f,0.4f,0.1f,1.0f);
    while (!glfwWindowShouldClose(win)) //Game loop.
    {
        glClear(GL_COLOR_BUFFER_BIT); //Constantly refresh the backgound color in accordance with the glClearColor().

        //Tell OpenGL which shader program to use, right before the mesh rendering. Since we only have 1 shader, we could
        //have placed the following command outside the while loop.
        glUseProgram(shader_prog);

        //Bind the vao before drawing : Automatically binds the associated vbo and vertex attribute setup. Basically we
        //tell OpenGL which mesh to render. Remember The core info is stored in the vbo. The vao is used to remember the configuration of the vbo.
        //Since we only have 1 mesh, we could have skipped the following command because we already binded the vao above, when we generated it.
        //glBindVertexArray(vao); //Optional.
        
        //Actually plot the mesh.
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    //Free resources.
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    
    glDeleteProgram(shader_prog);
    glDeleteShader(vshader);
    glDeleteShader(fshader);

    glfwTerminate();
    return 0;
}
