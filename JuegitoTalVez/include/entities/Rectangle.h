#pragma once
#include "core/Renderer.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Rectangle{
    protected:
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec2 size;
    glm::vec3 color;

    public:
    Rectangle(const glm::vec2& pos, const glm::vec2& sz, const glm::vec2& vel, const glm::vec3& col);
    void render(Renderer& Renderer, unsigned int shaderProgram, const glm::mat4& projectionView);
    glm::vec2 getPosition() const { return position; }
    glm::vec2 getSize() const { return size; }
};