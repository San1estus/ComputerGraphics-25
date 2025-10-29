#include "entities/Rectangle.h"

Rectangle::Rectangle(const glm::vec2& pos, const glm::vec2& sz, const glm::vec2& vel, const glm::vec3& col): position(pos), size(sz), velocity(vel), color(col){}

void Rectangle::render(Renderer& renderer, unsigned int shaderProgram, const glm::mat4& projectionView){
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));
    model = glm::scale(model, glm::vec3(size, 1.0f));

    glm::mat4 mvp = projectionView * model;

    renderer.draw(shaderProgram, mvp, model, color);
}