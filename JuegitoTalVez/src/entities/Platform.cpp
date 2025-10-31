#include "entities/Platform.h"

Platform::Platform(const glm::vec2& pos, const glm::vec2& sz, Texture* texture): Rectangle(pos, sz, glm::vec2(0.0f), texture){}

bool Platform::checkCollision(const glm::vec2& playerPos, const glm::vec2& playerSize){
    bool collisionX = playerPos.x + playerSize.x >= position.x &&
                      position.x + size.x >= playerPos.x;
    bool collisionY = playerPos.y + playerSize.y >= position.y &&
                      position.y + size.y >= playerPos.y;
    return collisionX && collisionY;
}