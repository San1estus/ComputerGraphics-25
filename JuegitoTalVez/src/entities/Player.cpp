#include "entities/Player.h"
#include "core/Window.h"
#include "entities/Platform.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Player::Player(const glm::vec2& pos,const glm::vec2& size, Texture* texture)
:Rectangle(pos, size, glm::vec2(0.0f), texture), onGround(false){}
static float maxSpeed = 200.0f;
static float minSpeed = -200.0f;

void Player::handleInput(const Window& window){
    float speed = 1.0f;

    if(window.isKeyPressed(GLFW_KEY_A)) velocity.x -= speed;
    else if(window.isKeyPressed(GLFW_KEY_D)) velocity.x += speed; 
    else if(velocity.x > 0) velocity.x -= speed;
    else if(velocity.x < 0) velocity.x += speed;
    
    velocity.x = glm::clamp(velocity.x, minSpeed, maxSpeed);

    if(window.isKeyPressed(GLFW_KEY_W) && onGround){
        velocity.y += 350.0f;
        onGround = false; 
    }
}

void Player::update(float dt, const std::vector<Platform> platforms){
    velocity.y -= 150.0f * 2.5f*dt;
    
    position += velocity * dt;
    
    position.x = glm::clamp(position.x, 0.0f, 1890.0f);
    if(position.x == 0.0f || position.x == 1890.0f) velocity.x = 0.0f;
    onGround = false;
    for(Platform platform : platforms){
        if(platform.checkCollision(position, size)){
            position.y = platform.getPosition().y+platform.getSize().y;
            velocity.y = 0;
            onGround = true;
        }
    }
}
