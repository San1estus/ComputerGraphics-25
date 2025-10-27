#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Window{
    private:
    GLFWwindow* window;
    int width, heigth;
    const char* title;
    
    public:
    Window(int width, int height, const char* title);
    bool init();
    void swapBuffers();
    void pollEvent();
    void shouldClose() const;
    void close();
    void destroy();
    void isKeyPressed(int key) const;
};