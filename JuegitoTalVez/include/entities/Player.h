#pragma once
#include "core/Renderer.h"
#include "Rectangle.h"
#include <vector>

class Platform;
class Player : public Rectangle{
    private:
    bool onGround = false;

    public:

    Player(const glm::vec2& pos,const glm::vec2& size, Texture* texture);
    void handleInput(const class Window& window);
    void update(float dt, const std::vector<Platform> platforms);
};