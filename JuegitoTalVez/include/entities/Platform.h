#pragma once
#include "core/Renderer.h"
#include "core/Shader.h"
#include "Rectangle.h"
#include <glm/glm.hpp>
#include <vector>

class Platform : public Rectangle{
    public:
    Platform(const glm::vec2& pos, const glm::vec2& sz);
    bool checkCollision(const glm::vec2& position, const glm::vec2& size);
};