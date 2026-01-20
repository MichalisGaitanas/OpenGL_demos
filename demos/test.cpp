#include <GLFW/glfw3.h>
#include <cmath>
#include <thread>
#include <chrono>

// Helper to force the OS to notice the change
void forceDraw() {
    glFlush();
    // This tiny sleep is the "secret sauce" to break modern smoothing.
    // It gives the OS Compositor a window to grab a half-finished frame.
    std::this_thread::sleep_for(std::chrono::milliseconds(2)); 
}

int main() {
    if (!glfwInit()) return -1;

    // Explicitly disable double buffering
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "Single Buffer Visual Artifacts", NULL, NULL);
    glfwMakeContextCurrent(window);

    glClearColor(1.0f,1.0f,1.0f,1.0f);
    while (!glfwWindowShouldClose(window)) {
        // 1. CLEAR: The screen goes black
        glClear(GL_COLOR_BUFFER_BIT);
        forceDraw(); 

        float x = sin(glfwGetTime()) * 0.5f;

        // 2. DRAW PIECE 1: Only one line appears
        glBegin(GL_LINES);
            glColor3f(1.0f, 0.0f, 0.0f);
            glVertex2f(x - 0.4f, -0.4f);
            glVertex2f(x + 0.4f, -0.4f);
        glEnd();
        forceDraw();

        // 3. DRAW PIECE 2: The rest of the triangle appears
        glBegin(GL_TRIANGLES);
            glColor3f(1.0f, 0.0f, 0.0f);
            glVertex2f(x - 0.4f, -0.4f);
            glVertex2f(x + 0.4f, -0.4f);
            glVertex2f(x, 0.4f);
        glEnd();
        forceDraw();

        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}