#include "core/Game.h"

static const float baseRectangle[]{
    0.3f, 0.3f, 0.0f,
    0.3f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, 0.3f, 0.0f
};

static const unsigned int baseIndices[]{
    0, 1, 3, 
    1, 2, 3
};

Game::Game(int width, int height, const char* title)
: window(width, height, title), shader(), player(glm::vec2(210.0f) ,glm::vec2(20.0f, 20.0f)), renderer(){}

bool Game::init(){
    if (!window.init()) return false;

    shader.init("res/shaders/vertex.vs", "res/shaders/fragment.fs");
    renderer.init();

    platforms.emplace_back(glm::vec2(0.0f, 0.0f), glm::vec2(1920.0f, 200.0f));  // Suelo
    
    // Plataformas
    platforms.emplace_back(glm::vec2(200.0f, 300.0f), glm::vec2(150.0f, 30.0f)); 
    platforms.emplace_back(glm::vec2(400.0f, 300.0f), glm::vec2(150.0f, 30.0f));

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
    player.handleInput(window);

    if(glfwGetKey(window.getHandle(), GLFW_KEY_ESCAPE) == GLFW_PRESS){
        window.close();
    }
}

void Game::render(){
    glClearColor(0.0f,0.0f,0.0f,0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shader.use();
    player.render(renderer, shader.ID, projectionView);
    for(auto& platform : platforms){
        platform.render(renderer, shader.ID, projectionView);
    }
}

void Game::update(){
    player.update(deltaTime, platforms);
}



Game::~Game(){
    renderer.~Renderer();
}