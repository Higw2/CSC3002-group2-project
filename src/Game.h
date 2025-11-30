#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <glm/glm.hpp>
#include "TiledMap.h"
#include "Player.h"
#include "Camera.h"
#include "StartMenu.h"

class Game {
public:
    Game();
    ~Game();
    bool init();
    void run();
    glm::vec2 getPlayerPosition() const { return player->getPosition(); }

    enum GameState {
        STATE_MENU,
        STATE_PLAYING,
        STATE_PAUSED,
        STATE_DEATH_ANIMATION
    };

private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    bool isRunning = false;
    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 350;

    // 时间管理变量
    Uint32 lastUpdateTime = 0;
    float deltaTime = 0.016f;
    const float MAX_DELTA_TIME = 0.05f;

    TiledMap* map = nullptr;
    Player* player = nullptr;
    Camera* camera = nullptr;
    StartMenu* startMenu = nullptr;
    GameState gameState = STATE_MENU;

    // 死亡图片相关
    SDL_Texture* deathImage = nullptr;
    Uint32 deathStartTime = 0;
    const Uint32 DEATH_DISPLAY_TIME = 2000; // 2秒

    void handleEvents();
    void update(float deltaTime);
    void render();
    void runMenuState();
    void runPlayingState();
    void runDeathAnimationState();
    void startNewGame();
    void handlePlayerDeath();
    bool loadDeathImage(); // 加载死亡图片
    void cleanupDeathImage(); // 清理死亡图片资源
};