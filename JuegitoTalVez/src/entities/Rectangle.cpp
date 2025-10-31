#include "entities/Rectangle.h"

Rectangle::Rectangle(const glm::vec2& pos, const glm::vec2& sz, const glm::vec2& vel, Texture* texture): position(pos), size(sz), velocity(vel), texture(texture){}

void Rectangle::render(Renderer& renderer, Shader& shader, const glm::mat4& projectionView){
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));
    model = glm::scale(model, glm::vec3(size, 1.0f));

    glm::mat4 mvp = projectionView * model;

    if(texture){
        texture->Bind(0);
        shader.setInt("u_Texture", 0);
    }

    renderer.draw(shader.ID, mvp, model);
}