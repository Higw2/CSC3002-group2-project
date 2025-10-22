#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <glm/glm.hpp>
#include "TiledMap.h"
#include "Player.h"
#include "Camera.h"

class Game {
public:
    Game();
    ~Game();
    bool init();
    void run();
    glm::vec2 getPlayerPosition() const { return player->getPosition(); }

private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    bool isRunning = false;
    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 350;

    TiledMap* map = nullptr;
    Player* player = nullptr;
    Camera* camera = nullptr;

    void handleEvents();
    void update();
    void render();
};