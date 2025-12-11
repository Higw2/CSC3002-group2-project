#include "Game.h"

#include <iostream>
#include <string>

Game::Game() : deathImage(nullptr), winImage(nullptr) {}

Game::~Game() {
    cleanupHudText();
    if (hudFont) {
        TTF_CloseFont(hudFont);
        hudFont = nullptr;
    }
    delete map;
    delete player;
    delete camera;
    delete startMenu;
    cleanupDeathImage();
    cleanupWinImage();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    Mix_Quit();
    SDL_Quit();
}

bool Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL 初始化失败: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow(
        "EchoRidge", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!window) {
        std::cerr << "创建窗口失败: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "创建渲染器失败: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image 初始化失败: " << IMG_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    if (!audioManager.init()) {
        std::cerr << "音频系统初始化失败" << std::endl;
    } else {
        loadAllSounds();
    }

    startMenu = new StartMenu(renderer);
    if (!startMenu->init()) {
        std::cerr << "开始菜单初始化失败" << std::endl;
    }

    if (!loadDeathImage()) {
        std::cerr << "死亡图片加载失败，使用纯色背景替代" << std::endl;
    }

    if (!loadWinImage()) {
        std::cerr << "通关图片加载失败，使用纯色背景替代" << std::endl;
    }

    gameState = STATE_MENU;
    lastUpdateTime = SDL_GetTicks();
    std::cout << "时间系统初始化完成 LastUpdateTime: " << lastUpdateTime << std::endl;

    initHudFont();
    isRunning = true;
    return true;
}

void Game::loadAllSounds() {
    audioManager.loadSound("die", "assets/sounds/die.wav");
    audioManager.loadSound("win", "assets/sounds/win.wav");
    audioManager.loadSound("coin", "assets/sounds/coin.wav");
    audioManager.loadSound("jump", "assets/sounds/jump.wav");
    audioManager.loadSound("hurt", "assets/sounds/hurt.wav");

    audioManager.setMasterVolume(80);
    audioManager.setMusicVolume(50);
    audioManager.setVolume("jump", 120);
    audioManager.setVolume("hurt", 120);
    audioManager.setVolume("coin", 100);
    audioManager.setVolume("die", 110);
    audioManager.setVolume("win", 110);

    std::cout << "音频加载完成，跳跃/受伤音量 120，背景音乐 50" << std::endl;
}

void Game::startMenuMusic() {
    if (!menuMusicStarted) {
        audioManager.stopMusic();
        audioManager.playMusic("assets/sounds/begining.wav", -1);
        menuMusicStarted = true;
        gameMusicStarted = false;
        std::cout << "开始播放菜单音乐 (begining.wav)" << std::endl;
    }
}

void Game::startGameMusic() {
    if (!gameMusicStarted) {
        audioManager.stopMusic();
        audioManager.playMusic("assets/sounds/main.wav", -1);
        gameMusicStarted = true;
        menuMusicStarted = false;
        std::cout << "开始播放游戏音乐 (main.wav)" << std::endl;
    }
}

void Game::stopAllMusic() {
    audioManager.stopMusic();
    menuMusicStarted = false;
    gameMusicStarted = false;
}

void Game::runMenuState() {
    if (!menuMusicStarted) {
        startMenuMusic();
    }

    if (startMenu) {
        startMenu->reset();
        int choice = startMenu->run();

        switch (choice) {
            case 0:
                std::cout << "开始新游戏" << std::endl;
                startGameMusic();
                startNewGame();
                gameState = STATE_PLAYING;
                break;
            case 1:
                std::cout << "继续游戏" << std::endl;
                if (map && player && camera) {
                    startGameMusic();
                    gameState = STATE_PLAYING;
                } else {
                    startGameMusic();
                    startNewGame();
                    gameState = STATE_PLAYING;
                }
                break;
            case 2:
                std::cout << "退出游戏" << std::endl;
                isRunning = false;
                break;
        }
    } else {
        std::cout << "菜单不可用，直接开始游戏" << std::endl;
        startGameMusic();
        startNewGame();
        gameState = STATE_PLAYING;
    }
}

void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            isRunning = false;
        } else if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                if (gameState == STATE_PLAYING) {
                    gameState = STATE_MENU;
                    std::cout << "回到开始菜单" << std::endl;
                    audioManager.stopAll();
                    startMenuMusic();
                }
            }
        }
    }
}

void Game::runPlayingState() {
    handleEvents();
    update(deltaTime);
    render();

    Uint32 frameTime = SDL_GetTicks() - lastUpdateTime;
    if (frameTime < 16) {
        SDL_Delay(16 - frameTime);
    }
}

void Game::update(float delta) {
    if (gameState != STATE_PLAYING) {
        return;
    }

    player->handleInput();

    player->setAudioCallback([this](const std::string& soundName) {
        audioManager.playSound(soundName);
        std::cout << "播放音效: " << soundName << std::endl;
    });

    player->update(*map, delta);

    bool coinCollected = coins.updateOnPlayerCollision(player->getWorldRect(), *map, score);
    if (coinCollected) {
        audioManager.playSound("coin");
    }

    camera->follow(player->getPosition(), map->getRenderScale());

    if (player->isDead()) {
        std::cout << "检测到玩家死亡" << std::endl;
        audioManager.playSound("die");
        stopAllMusic();
        handlePlayerDeath();
        return;
    }

    glm::vec2 playerPos = player->getPosition();
    camera->follow(playerPos, map->getRenderScale());

    if (playerPos.y >= 480.0f) {
        bool forceLock = true;
        camera->setLockedCenterY(512.0f, forceLock);
    }

    if (playerPos.x >= 4600.0f) {
        std::cout << "玩家到达终点 (" << playerPos.x << ", " << playerPos.y << ")" << std::endl;
        audioManager.playSound("win");
        stopAllMusic();
        handlePlayerWin();
        return;
    }
}

void Game::runDeathAnimationState() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            isRunning = false;
            return;
        } else if (e.type == SDL_KEYDOWN || e.type == SDL_MOUSEBUTTONDOWN) {
            std::cout << "玩家跳过死亡动画" << std::endl;
            gameState = STATE_MENU;
            if (player) {
                player->respawn();
            }
            audioManager.stopAll();
            startMenuMusic();
            return;
        }
    }

    Uint32 currentTime = SDL_GetTicks();
    Uint32 elapsed = currentTime - deathStartTime;

    static int debugCount = 0;
    if (debugCount++ % 60 == 0) {
        std::cout << "死亡动画进度: " << elapsed << "/" << DEATH_DISPLAY_TIME << " ms" << std::endl;
    }

    if (elapsed >= DEATH_DISPLAY_TIME) {
        std::cout << "死亡动画播放结束，回到菜单" << std::endl;
        gameState = STATE_MENU;
        if (player) {
            player->respawn();
        }
        audioManager.stopAll();
        startMenuMusic();
        return;
    }

    SDL_RenderClear(renderer);

    if (deathImage) {
        int imgWidth, imgHeight;
        SDL_QueryTexture(deathImage, nullptr, nullptr, &imgWidth, &imgHeight);

        SDL_Rect destRect;

        if (imgWidth > SCREEN_WIDTH || imgHeight > SCREEN_HEIGHT) {
            float scaleX = static_cast<float>(SCREEN_WIDTH) / imgWidth;
            float scaleY = static_cast<float>(SCREEN_HEIGHT) / imgHeight;
            float scale = std::min(scaleX, scaleY) * 0.8f;

            int scaledWidth = static_cast<int>(imgWidth * scale);
            int scaledHeight = static_cast<int>(imgHeight * scale);

            destRect = {(SCREEN_WIDTH - scaledWidth) / 2,
                        (SCREEN_HEIGHT - scaledHeight) / 2,
                        scaledWidth,
                        scaledHeight};
        } else {
            destRect = {(SCREEN_WIDTH - imgWidth) / 2,
                        (SCREEN_HEIGHT - imgHeight) / 2,
                        imgWidth,
                        imgHeight};
        }

        SDL_RenderCopy(renderer, deathImage, nullptr, &destRect);
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect screenRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderFillRect(renderer, &screenRect);
    }

    SDL_RenderPresent(renderer);
    SDL_Delay(16);
}

void Game::runWinAnimationState() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            isRunning = false;
            return;
        } else if (e.type == SDL_KEYDOWN || e.type == SDL_MOUSEBUTTONDOWN) {
            std::cout << "玩家跳过通关动画" << std::endl;
            gameState = STATE_MENU;
            std::cout << "最终得分 " << score << std::endl;
            audioManager.stopAll();
            startMenuMusic();
            return;
        }
    }

    Uint32 currentTime = SDL_GetTicks();
    Uint32 elapsed = currentTime - winStartTime;

    static int debugCount = 0;
    if (debugCount++ % 60 == 0) {
        std::cout << "通关动画进度: " << elapsed << "/" << WIN_DISPLAY_TIME << " ms" << std::endl;
    }

    if (elapsed >= WIN_DISPLAY_TIME) {
        std::cout << "通关动画播放结束，回到菜单" << std::endl;
        std::cout << "最终得分 " << score << std::endl;
        gameState = STATE_MENU;
        audioManager.stopAll();
        startMenuMusic();
        return;
    }

    SDL_RenderClear(renderer);

    if (winImage) {
        int imgWidth, imgHeight;
        SDL_QueryTexture(winImage, nullptr, nullptr, &imgWidth, &imgHeight);

        SDL_Rect destRect;

        if (imgWidth > SCREEN_WIDTH || imgHeight > SCREEN_HEIGHT) {
            float scaleX = static_cast<float>(SCREEN_WIDTH) / imgWidth;
            float scaleY = static_cast<float>(SCREEN_HEIGHT) / imgHeight;
            float scale = std::min(scaleX, scaleY) * 0.8f;

            int scaledWidth = static_cast<int>(imgWidth * scale);
            int scaledHeight = static_cast<int>(imgHeight * scale);

            destRect = {(SCREEN_WIDTH - scaledWidth) / 2,
                        (SCREEN_HEIGHT - scaledHeight) / 2,
                        scaledWidth,
                        scaledHeight};
        } else {
            destRect = {(SCREEN_WIDTH - imgWidth) / 2,
                        (SCREEN_HEIGHT - imgHeight) / 2,
                        imgWidth,
                        imgHeight};
        }

        SDL_RenderCopy(renderer, winImage, nullptr, &destRect);
    } else {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_Rect screenRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderFillRect(renderer, &screenRect);
    }

    SDL_RenderPresent(renderer);
    SDL_Delay(16);
}

void Game::handlePlayerWin() {
    std::cout << "玩家通关！恭喜" << std::endl;
    std::cout << "最终得分 " << score << std::endl;

    winStartTime = SDL_GetTicks();
    gameState = STATE_WIN_ANIMATION;
    std::cout << "切换到 STATE_WIN_ANIMATION" << std::endl;
}

bool Game::initHudFont() {
    if (hudFont) {
        return true;
    }
    if (TTF_WasInit() == 0) {
        if (TTF_Init() == -1) {
            std::cerr << "字体初始化失败: " << TTF_GetError() << std::endl;
            return false;
        }
    }
    hudFont = TTF_OpenFont("assets/fonts/FLyouzichati-Regular-2.ttf", 20);
    if (!hudFont) {
        hudFont = TTF_OpenFont("arial.ttf", 20);
    }
    if (!hudFont) {
        std::cerr << "字体加载失败: " << TTF_GetError() << std::endl;
        return false;
    }
    return true;
}

void Game::updateScoreTexture() {
    if (!hudFont) {
        return;
    }
    if (score == lastScoreRendered && scoreTexture) {
        return;
    }

    if (scoreTexture) {
        SDL_DestroyTexture(scoreTexture);
        scoreTexture = nullptr;
    }

    std::string text = "Coins: " + std::to_string(score);
    SDL_Color color{255, 215, 0, 255};
    SDL_Surface* surface = TTF_RenderUTF8_Blended(hudFont, text.c_str(), color);
    if (!surface) {
        std::cerr << "HUD文本表面创建失败: " << TTF_GetError() << std::endl;
        return;
    }
    scoreTexture = SDL_CreateTextureFromSurface(renderer, surface);
    scoreTexW = surface->w;
    scoreTexH = surface->h;
    SDL_FreeSurface(surface);
    if (!scoreTexture) {
        std::cerr << "HUD文本纹理创建失败: " << SDL_GetError() << std::endl;
        return;
    }
    lastScoreRendered = score;
}

void Game::renderHud() {
    if (!hudFont) {
        return;
    }
    updateScoreTexture();
    if (!scoreTexture) {
        return;
    }
    SDL_Rect dest;
    dest.w = scoreTexW;
    dest.h = scoreTexH;
    dest.x = SCREEN_WIDTH - dest.w - 12;
    if (dest.x < 0) {
        dest.x = 0;
    }
    dest.y = 8;
    SDL_RenderCopy(renderer, scoreTexture, nullptr, &dest);
}

void Game::cleanupHudText() {
    if (scoreTexture) {
        SDL_DestroyTexture(scoreTexture);
        scoreTexture = nullptr;
    }
}

void Game::handlePlayerDeath() {
    std::cout << "处理玩家死亡" << std::endl;

    if (player && player->isDead()) {
        std::cout << "玩家死亡，进入死亡动画" << std::endl;
        deathStartTime = SDL_GetTicks();
        gameState = STATE_DEATH_ANIMATION;
        std::cout << "切换到 STATE_DEATH_ANIMATION" << std::endl;
    }
}

void Game::startNewGame() {
    if (map) {
        delete map;
        map = nullptr;
    }
    if (player) {
        delete player;
        player = nullptr;
    }
    if (camera) {
        delete camera;
        camera = nullptr;
    }

    score = 0;
    lastScoreRendered = -1;
    cleanupHudText();

    try {
        map = new TiledMap("assets/maps/level1.tmj", renderer);
    } catch (const std::exception& e) {
        std::cerr << "地图加载失败: " << e.what() << std::endl;
        return;
    }

    map->setRenderScale(1.0f);

    
    std::cout << "==================================== 地图信息 ===========================================" << std::endl;
    std::cout << "=====================================================================================" << std::endl;
    std::cout << "=====================================================================================" << std::endl;
    std::cout << "=====================================================================================" << std::endl;
    std::cout << "原始尺寸: " << map->getMapPixelWidth() << "x" << map->getMapPixelHeight() << std::endl;
    std::cout << "内容尺寸: " << map->getContentPixelWidth() << "x" << map->getContentPixelHeight() << std::endl;
    std::cout << "瓦片尺寸: " << map->getTileWidth() << "x" << map->getTileHeight() << std::endl;
    std::cout << "屏幕尺寸: " << SCREEN_WIDTH << "x" << SCREEN_HEIGHT << std::endl;
    std::cout << "=====================================================================================" << std::endl;
    std::cout << "=====================================================================================" << std::endl;
    std::cout << "=====================================================================================" << std::endl;

    camera = new Camera(
        SCREEN_WIDTH, SCREEN_HEIGHT, map->getContentPixelWidth(), map->getContentPixelHeight());

    player = new Player(renderer);

    float startX = 100.0f;
    float startY = 100.0f;
    player->setPosition({startX, startY});

    if (!coins.load(renderer, "assets/coin.png")) {
        SDL_Log("加载失败assets/coin.png");
    }

    std::vector<SDL_FPoint> coinSpawns = map->getCoinPositions();
    std::cout << "[Coins] spawn count: " << coinSpawns.size() << std::endl;
    coins.spawnFixed(coinSpawns, map->getTileWidth());

    std::cout << "玩家初始位置: (" << startX << ", " << startY << ")" << std::endl;

    camera->follow(player->getPosition(), 1.0f);

    std::cout << "新游戏初始化完成" << std::endl;
    std::cout << "相机初始位置: (" << camera->x << ", " << camera->y << ")" << std::endl;
}

bool Game::loadDeathImage() {
    deathImage = IMG_LoadTexture(renderer, "assets/animations/you_die.png");
    if (!deathImage) {
        std::cerr << "无法加载死亡图片: " << IMG_GetError() << std::endl;
        std::cerr << "文件你确定再这里吗 assets/animations/you_die.png" << std::endl;
        return false;
    }
    std::cout << "死亡图片加载成功" << std::endl;
    return true;
}

bool Game::loadWinImage() {
    winImage = IMG_LoadTexture(renderer, "assets/animations/you_win.png");
    if (!winImage) {
        std::cerr << "无法加载通关图片: " << IMG_GetError() << std::endl;
        std::cerr << "把文件放在assets/animations/you_win.png" << std::endl;
        return false;
    }
    std::cout << "通关图片加载成功" << std::endl;
    return true;
}

void Game::cleanupDeathImage() {
    if (deathImage) {
        SDL_DestroyTexture(deathImage);
        deathImage = nullptr;
    }
}

void Game::cleanupWinImage() {
    if (winImage) {
        SDL_DestroyTexture(winImage);
        winImage = nullptr;
    }
}

void Game::render() {
    SDL_RenderClear(renderer);

    if (gameState == STATE_PLAYING) {
        map->renderBackground(renderer, *camera);
        map->renderTiles(renderer, *camera);
        coins.render(renderer, *camera, map->getRenderScale());
        player->render(renderer, *camera, map->getRenderScale());
        renderHud();
    }

    SDL_RenderPresent(renderer);
}

void Game::run() {
    if (!init()) {
        std::cerr << "初始化失败，程序终止" << std::endl;
        return;
    }

    std::cout << "游戏初始化成功，开始主循环" << std::endl;

    while (isRunning) {
        Uint32 currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastUpdateTime) / 1000.0f;
        lastUpdateTime = currentTime;

        if (deltaTime > MAX_DELTA_TIME) {
            std::cout << "帧时间过长 " << deltaTime * 1000 << "ms, 限制到 "
                      << MAX_DELTA_TIME * 1000 << "ms" << std::endl;
            deltaTime = MAX_DELTA_TIME;
        }

        static Uint32 frameCount = 0;
        static Uint32 lastFpsTime = 0;
        frameCount++;
        if (currentTime - lastFpsTime >= 1000) {
            float fps = frameCount / ((currentTime - lastFpsTime) / 1000.0f);
            std::cout << "FPS: " << fps << ", DeltaTime: " << deltaTime * 1000 << "ms" << std::endl;
            frameCount = 0;
            lastFpsTime = currentTime;
        }

        switch (gameState) {
            case STATE_MENU:
                runMenuState();
                break;
            case STATE_PLAYING:
                runPlayingState();
                break;
            case STATE_PAUSED:
                runPlayingState();
                break;
            case STATE_DEATH_ANIMATION:
                runDeathAnimationState();
                break;
            case STATE_WIN_ANIMATION:
                runWinAnimationState();
                break;
        }
    }
}
