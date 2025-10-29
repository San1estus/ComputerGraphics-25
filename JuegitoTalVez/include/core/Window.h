#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Window{
    private:
    GLFWwindow* window;
    int width, height;
    const char* title;
    
    public:
    Window(int width, int height, const char* title);
    bool init();
    void swapBuffers();
    void pollEvent();
    bool shouldClose() const;
    void close();
    void destroy();
    bool isKeyPressed(int key) const;
    int getWidth(){return width;}
    int getHeight(){return height;}
    GLFWwindow* getHandle(){return window;}
};