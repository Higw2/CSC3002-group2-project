#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <glm/glm.hpp>
#include "TiledMap.h"
#include "Player.h"
#include "Camera.h"
#include "StartMenu.h"
#include "Coin.hpp"
#include "AudioManager.h"

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
        STATE_WIN_ANIMATION
    };

private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    bool isRunning = false;
    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 350;

    AudioManager audioManager;
    
    bool menuMusicStarted = false;
    bool gameMusicStarted = false;
    
    Uint32 lastUpdateTime = 0;
    float deltaTime = 0.016f;
    const float MAX_DELTA_TIME = 0.05f;

    TiledMap* map = nullptr;
    Player* player = nullptr;
    Camera* camera = nullptr;
    StartMenu* startMenu = nullptr;
    GameState gameState = STATE_MENU;

    SDL_Texture* deathImage = nullptr;
    Uint32 deathStartTime = 0;
    const Uint32 DEATH_DISPLAY_TIME = 200000; 

    SDL_Texture* winImage = nullptr;
    Uint32 winStartTime = 0;
    const Uint32 WIN_DISPLAY_TIME = 300000;

    CoinManager coins;
    int score = 0;

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
    void runWinAnimationState();
    void startNewGame();
    void handlePlayerDeath();
    void handlePlayerWin();
    bool loadDeathImage();
    bool loadWinImage();
    void cleanupDeathImage();
    void cleanupWinImage();
    
    void loadAllSounds();
    void startMenuMusic();
    void startGameMusic();
    void stopAllMusic();
};