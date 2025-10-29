#include "core/Game.h"
#include <iostream>

const int SCREEN_WIDTH = 1080;
const int SCREEN_HEIGHT = 1920;
int main(void){

    Game game(1920, 1080, "2D Platformer");

    try {
        if (!game.init()) {
            std::cerr << "Fallo al inicializar el juego." << std::endl;
            return -1;
        }

        game.run();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }   
    
    game.~Game();
    return 0;
}