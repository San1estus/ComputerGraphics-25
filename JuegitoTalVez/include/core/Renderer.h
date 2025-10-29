#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Renderer{
    private:
    unsigned int VAO, VBO, EBO;
    unsigned int indexCount;

    public:
    Renderer();
    void init();

    void draw(unsigned int shaderProgram, const glm::mat4& mvp, const glm::mat4& model, const glm::vec3& color);

    ~Renderer();
};