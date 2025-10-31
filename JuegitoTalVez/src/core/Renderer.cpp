#include "core/Renderer.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

Renderer::Renderer() : VAO(0), VBO(0), EBO(0), indexCount(0) {};
void Renderer::init(){
    static const float vertices[]{
    0.3f, 0.3f, 0.0f, 0.3f, 0.3f,
    0.3f, 0.0f, 0.0f, 0.3f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.3f, 0.0f, 0.0f, 0.3f
};

static const unsigned int indices[]{
    0, 1, 3, 
    1, 2, 3
};
    
    indexCount = 6;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}
void Renderer::draw(unsigned int shaderProgram, const glm::mat4& mvp, const glm::mat4& model){
    int mvpLocation = glGetUniformLocation(shaderProgram, "u_MVP"); 
    int modelLocation = glGetUniformLocation(shaderProgram, "u_Model"); 

    glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

Renderer::~Renderer() {
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}