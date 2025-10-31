#pragma once
#include <GL/glew.h>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Shader{
    public:
    unsigned int ID;

    Shader();
    void init(std::string vertexPath, std::string fragmentPath);
    void use() const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
    void setBool(const std::string &name, const bool values) const;
    void setInt(const std::string &name, const int values) const;
    void setFloat(const std::string &name, const float values) const;

    private:
    unsigned int CompileShader(unsigned int type, const std::string source);
};