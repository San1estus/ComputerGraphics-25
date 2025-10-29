#include "core/Window.h"
#include <iostream>

Window::Window(int width, int height, const char* title): width(width), height(height), title(title), window(nullptr){}

bool Window::init(){
    if (!glfwInit())
        return -1;
    window = glfwCreateWindow(1920, 1080, "The Puppet", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cout << "Error" << std::endl;
        return false;
    }

    glViewport(0,0,width, height);
    return true;
}

void Window::swapBuffers(){glfwSwapBuffers(window);}
void Window::pollEvent(){glfwPollEvents();}
bool Window::shouldClose() const {return glfwWindowShouldClose(window);}
void Window::close(){glfwSetWindowShouldClose(window, true);}
void Window::destroy(){glfwTerminate();}
bool Window::isKeyPressed(int key) const{return glfwGetKey(window, key) == GLFW_PRESS;}