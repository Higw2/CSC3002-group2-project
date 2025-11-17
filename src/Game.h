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

    // 添加时间管理变量
    Uint32 lastUpdateTime = 0;    // 上一帧的时间戳（毫秒）
    float deltaTime = 0.016f;     // 时间增量（秒），初始化为16ms
    const float MAX_DELTA_TIME = 0.05f; // 最大时间增量，避免卡顿时的物理异常

    TiledMap* map = nullptr;
    Player* player = nullptr;
    Camera* camera = nullptr;

    void handleEvents();
    void update(float deltaTime);  // 修改：添加deltaTime参数
    void render();
};
