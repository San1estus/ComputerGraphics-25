#pragma once
#include "Window.h"
#include "Renderer.h"
#include "Shaders.h"
#include "entities/Player.h"
#include <vector>

class Game{
    private:

    Window window;
    Renderer renderer;
    Shader shader;
    Player player;
};