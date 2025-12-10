#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <glm/glm.hpp>
#include "TiledMap.h"
#include "Player.h"
#include "Camera.h"
#include "StartMenu.h"
#include "Coin.hpp"

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
        STATE_DEATH_ANIMATION,
        STATE_WIN_ANIMATION  // 新增：通关动画状态
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

    // 通关图片相关（新增）
    SDL_Texture* winImage = nullptr;
    Uint32 winStartTime = 0;
    const Uint32 WIN_DISPLAY_TIME = 3000; // 3秒（比死亡动画稍长）

    //金币系统部分
    CoinManager coins;       // 新增成员
    int score = 0;           // 简单计分

    // HUD
    TTF_Font* hudFont = nullptr;
    SDL_Texture* scoreTexture = nullptr;
    int scoreTexW = 0;
    int scoreTexH = 0;
    int lastScoreRendered = -1;
    bool initHudFont();
    void updateScoreTexture();
    void renderHud();
    void cleanupHudText();

    void handleEvents();
    void update(float deltaTime);
    void render();
    void runMenuState();
    void runPlayingState();
    void runDeathAnimationState();
    void runWinAnimationState();  // 新增：通关动画状态处理
    void startNewGame();
    void handlePlayerDeath();
    void handlePlayerWin();    // 新增：处理玩家通关
    bool loadDeathImage(); // 加载死亡图片
    bool loadWinImage();   // 新增：加载通关图片
    void cleanupDeathImage(); // 清理死亡图片资源
    void cleanupWinImage();   // 新增：清理通关图片资源
};
