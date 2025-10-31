#pragma once
#include <GL/glew.h>
#include <glfw/glfw3.h>
#include "Window.h"
#include "Renderer.h"
#include "Shader.h"
#include "entities/Player.h"
#include "entities/Platform.h"
#include <vector>
#include <memory>

class Game{
    private:

    Window window;
    Renderer renderer;
    Shader shader;
    std::unique_ptr<Player> player;

    glm::mat4 projectionView;

    std::vector<Platform> platforms;
    
    std::unique_ptr<Texture> PlayerTex;
    std::unique_ptr<Texture> PlatformTex;

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    public:
    Game(int width, int height, const char* title);
    bool init();
    void processInput();
    void update();
    void render();
    void run();
    ~Game();
};