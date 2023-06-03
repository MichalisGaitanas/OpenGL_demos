#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<cstdio>
#include<cmath>
#include"../include/shader.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include"../include/stb_image.h"

int width = 1000, height = 1000;
const char *label = "Rotating texture";

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
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    GLFWwindow *window = glfwCreateWindow(width, height, label, NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);
    int monitx, monity;
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    glfwGetMonitorPos(monitor, &monitx, &monity);
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    glfwSetWindowPos( window, (monitx + mode->width - width)/2, (monity + mode->height - height)/2 );
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    //validate glew
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }

                       //vertices (x,y,z),  textures (s,t)
    float texsquare[] = { -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,
                           0.5f, -0.5f, 0.0f,   1.0f, 0.0f,
                           0.5f,  0.5f, 0.0f,   1.0f, 1.0f,
                       
                          -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,
                           0.5f,  0.5f, 0.0f,   1.0f, 1.0f,
                          -0.5f,  0.5f, 0.0f,   0.0f, 1.0f };

    //tell OpenGL how to interpret the texsquarep[] data
    unsigned int vao,vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texsquare), texsquare, GL_STATIC_DRAW);
    //vertices
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0 );
    glEnableVertexAttribArray(0);
    //textures
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)) );
    glEnableVertexAttribArray(1);

    //load the image-texture
    int imgwidth, imgheight, imgchannels;
    const char *imgpath = "../images/texture/test_kimin.jpg";
    //const char *imgpath = "../images/texture/ground900x900.jpg";
    //const char *imgpath = "../images/texture/asteroid700x700.jpg";
    unsigned char *imgdata = stbi_load(imgpath, &imgwidth, &imgheight, &imgchannels, 0);
    if (!imgdata)
    {
        printf("'%s' not found. Exiting...\n", imgpath);
        stbi_image_free(imgdata); //free image resources
        exit(EXIT_FAILURE);
    }
    //stbi_set_flip_vertically_on_load(false);
    //tell OpenGL how to apply the texture
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imgwidth, imgheight, 0, GL_RGB, GL_UNSIGNED_BYTE, imgdata);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(imgdata); //free image resources

    //texture shader
    shader texshad("../shaders/vertex/trans_m_texture.vert","../shaders/fragment/texture.frag");
    texshad.use();

    glClearColor(0.0f,0.0f,0.0f,0.0f);
    while (!glfwWindowShouldClose(window))
    {
        raw_hardware_input(window);
        glClear(GL_COLOR_BUFFER_BIT);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f,0.0f,0.0f));
        texshad.set_mat4_uniform("model",model);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);

    glfwTerminate();
    return 0;
}
