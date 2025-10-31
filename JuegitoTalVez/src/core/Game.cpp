#include "core/Game.h"
#include <iostream>

Game::Game(int width, int height, const char* title)
: window(width, height, title), shader(), renderer(){}

bool Game::init(){
    if (!window.init()) return false;

    shader.init("res/shaders/vertex.vs", "res/shaders/fragment.fs");
    try {
        PlayerTex = std::make_unique<Texture>("textures/player.png");
        PlatformTex = std::make_unique<Texture>("textures/platform.png");
    } catch (const std::exception& e) {
        std::cerr << "Error al cargar texturas: " << e.what() << std::endl;
        return false;
    }
    player = std::make_unique<Player>(
        glm::vec2(100.0f, 300.0f), 
        glm::vec2(40.0f, 60.0f), 
        PlayerTex.get()
    );
    renderer.init();    

    platforms.emplace_back(glm::vec2(0.0f, 0.0f), glm::vec2(1920.0f, 200.0f), PlatformTex.get());  // Suelo
    
    // Plataformas
    platforms.emplace_back(glm::vec2(200.0f, 300.0f), glm::vec2(150.0f, 30.0f), PlatformTex.get()); 
    platforms.emplace_back(glm::vec2(400.0f, 300.0f), glm::vec2(150.0f, 30.0f), PlatformTex.get());

    glm::mat4 proj = glm::ortho(0.0f, (float)window.getWidth(), 0.0f, (float)window.getHeight(), -1.0f, 1.0f);
    glm::mat4 view = glm::mat4(1.0f);

    projectionView = proj * view;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return true;
}

void Game::run(){
    while(!window.shouldClose()){
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame- lastFrame;
        lastFrame = currentFrame;

        processInput();
        update();
        render();
        window.swapBuffers();
        window.pollEvent();
    }
}

void Game::processInput(){
    if(player)player->handleInput(window);

    if(glfwGetKey(window.getHandle(), GLFW_KEY_ESCAPE) == GLFW_PRESS){
        window.close();
    }
}

void Game::render(){
    glClearColor(0.0f,0.0f,0.0f,0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shader.use();
    if(player) player->render(renderer, shader, projectionView);
    for(auto& platform : platforms){
        platform.render(renderer, shader, projectionView);
    }
}

void Game::update(){
    if(player) player->update(deltaTime, platforms);
}



Game::~Game(){
}