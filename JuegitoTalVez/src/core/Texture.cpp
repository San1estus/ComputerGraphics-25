#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "core/Texture.h" 
#include <iostream>

Texture::Texture(const std::string& path)
    : m_RendererID(0), m_FilePath(path), m_Width(0), m_Height(0), m_BPP(0) {
    
    // OpenGL espera que las texturas empiecen desde abajo-izquierda (0,0)
    stbi_set_flip_vertically_on_load(1);

    // Cargamos la imagen (forzando 4 canales - RGBA)
    unsigned char* localBuffer = stbi_load(path.c_str(), &m_Width, &m_Height, &m_BPP, 4);

    if (localBuffer) {
        // Generamos la textura en OpenGL
        glGenTextures(1, &m_RendererID);
        glBindTexture(GL_TEXTURE_2D, m_RendererID);

        // Parámetros de la textura (básicos)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Subimos los datos a la GPU
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, localBuffer);
        
        // Desvinculamos la textura
        glBindTexture(GL_TEXTURE_2D, 0);

        // Liberamos la memoria de la CPU
        stbi_image_free(localBuffer);
    } else {
        std::cout << "Error: No se pudo cargar la textura: " << path << std::endl;
    }
}

Texture::~Texture() {
    glDeleteTextures(1, &m_RendererID);
}

void Texture::Bind(unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_RendererID);
}

void Texture::Unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}