#include "Game.h"
#include <iostream>

Game::Game() : deathImage(nullptr) {}

Game::~Game() {
    delete map;
    delete player;
    delete camera;
    delete startMenu;
    cleanupDeathImage();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

bool Game::loadDeathImage() {
    deathImage = IMG_LoadTexture(renderer, "assets/animations/you_die.png");
    if (!deathImage) {
        std::cerr << "无法加载死亡图片: " << IMG_GetError() << std::endl;
        std::cerr << "请确保文件存在: assets/animations/you_die.png" << std::endl;
        return false;
    }
    std::cout << "死亡图片加载成功" << std::endl;
    return true;
}

void Game::cleanupDeathImage() {
    if (deathImage) {
        SDL_DestroyTexture(deathImage);
        deathImage = nullptr;
    }
}

bool Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL初始化失败: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow(
        "EchoRidge",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        0
    );
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
        std::cerr << "SDL_image初始化失败: " << IMG_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    startMenu = new StartMenu(renderer);
    if (!startMenu->init()) {
        std::cerr << "开始菜单初始化失败" << std::endl;
    }

    if (!loadDeathImage()) {
        std::cerr << "死亡图片加载失败，将使用默认效果" << std::endl;
    }

    gameState = STATE_MENU;
    lastUpdateTime = SDL_GetTicks();
    std::cout << "时间系统初始化完成. LastUpdateTime: " << lastUpdateTime << std::endl;

    isRunning = true;
    return true;
}

void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            isRunning = false;
        }
        else if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                if (gameState == STATE_PLAYING) {
                    gameState = STATE_MENU;
                    std::cout << "返回开始菜单" << std::endl;
                }
            }
        }
    }
}

void Game::update(float deltaTime) {
    if (gameState != STATE_PLAYING) {
        return;
    }

    player->handleInput();
    player->update(*map, deltaTime);

    // 碰到金币则加分并让金币消失
    coins.updateOnPlayerCollision(player->getWorldRect(), score);
    camera->follow(player->getPosition(), map->getRenderScale());
    
    if (player->isDead()) {
        std::cout << "!!! 在update中检测到玩家死亡 !!!" << std::endl;
        handlePlayerDeath();
        return;
    }
    
    glm::vec2 playerPos = player->getPosition();
    camera->follow(playerPos, map->getRenderScale());

    // ===== 示例：一行启用/禁用 Y 轴中心锁定 =====
    // 这里作为演示我们强制开启测试开关，将相机中心 Y 锁定为 100。
    bool TEST_FORCE_LOCK = true; // 设为 true 则锁定，false 则解除锁定
    camera->setLockedCenterY(100.0f, TEST_FORCE_LOCK);
    // ============================================
}

void Game::render() {
    SDL_RenderClear(renderer);

    if (gameState == STATE_PLAYING) {
        map->renderBackground(renderer, *camera);
        map->renderTiles(renderer, *camera);
        coins.render(renderer, *camera, map->getRenderScale());
        player->render(renderer, *camera, map->getRenderScale());
    }

    SDL_RenderPresent(renderer);
}

void Game::run() {
    if (!init()) {
        std::cerr << "初始化失败，程序终止" << std::endl;
        return;
    }

    std::cout << "游戏初始化成功，开始主循环..." << std::endl;

    while (isRunning) {
        Uint32 currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastUpdateTime) / 1000.0f;
        lastUpdateTime = currentTime;

        if (deltaTime > MAX_DELTA_TIME) {
            std::cout << "帧时间过长: " << deltaTime * 1000 << "ms, 限制到" 
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
        }
    }
}

void Game::runMenuState() {
    if (startMenu) {
        startMenu->reset();
        int choice = startMenu->run();
        
        switch (choice) {
            case 0:
                std::cout << "开始新游戏" << std::endl;
                startNewGame();
                gameState = STATE_PLAYING;
                break;
            case 1:
                std::cout << "继续游戏" << std::endl;
                if (map && player && camera) {
                    gameState = STATE_PLAYING;
                } else {
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
        startNewGame();
        gameState = STATE_PLAYING;
    }
}

void Game::runPlayingState() {
    handleEvents();
    update(deltaTime);
    
    // 临时调试：检查坐标系统
    static int coordDebugCount = 0;
    if (coordDebugCount++ % 120 == 0 && player && camera) {
        glm::vec2 playerPos = player->getPosition();
        const SDL_Rect& view = camera->getView();
        
        std::cout << "=== 坐标系统检查 ===" << std::endl;
        std::cout << "玩家世界坐标: (" << playerPos.x << ", " << playerPos.y << ")" << std::endl;
        std::cout << "相机位置: (" << view.x << ", " << view.y << ")" << std::endl;
        std::cout << "地图缩放: " << map->getRenderScale() << std::endl;
        
        // 计算玩家应该在屏幕上的位置
        float screenX = playerPos.x * map->getRenderScale() - view.x;
        float screenY = playerPos.y * map->getRenderScale() - view.y;
        std::cout << "玩家屏幕位置计算: (" << screenX << ", " << screenY << ")" << std::endl;
        std::cout << "是否在屏幕内: " 
                  << (screenX >= 0 && screenX <= SCREEN_WIDTH && screenY >= 0 && screenY <= SCREEN_HEIGHT ? "是" : "否") 
                  << std::endl;
    }
    
    render();

    Uint32 frameTime = SDL_GetTicks() - lastUpdateTime;
    if (frameTime < 16) {
        SDL_Delay(16 - frameTime);
    }
}

void Game::runDeathAnimationState() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            isRunning = false;
            return;
        } else if (e.type == SDL_KEYDOWN || e.type == SDL_MOUSEBUTTONDOWN) {
            std::cout << "用户跳过死亡动画" << std::endl;
            gameState = STATE_MENU;
            if (player) {
                player->respawn();
            }
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
        std::cout << "死亡动画播放完毕，返回菜单" << std::endl;
        gameState = STATE_MENU;
        if (player) {
            player->respawn();
        }
        return;
    }

    SDL_RenderClear(renderer);
    
    if (deathImage) {
        int imgWidth, imgHeight;
        SDL_QueryTexture(deathImage, nullptr, nullptr, &imgWidth, &imgHeight);
        
        std::cout << "死亡图片尺寸: " << imgWidth << "x" << imgHeight << std::endl;
        std::cout << "屏幕尺寸: " << SCREEN_WIDTH << "x" << SCREEN_HEIGHT << std::endl;
        
        SDL_Rect destRect;
        
        // 如果图片比屏幕大，进行缩放
        if (imgWidth > SCREEN_WIDTH || imgHeight > SCREEN_HEIGHT) {
            // 计算缩放比例，保持宽高比
            float scaleX = (float)SCREEN_WIDTH / imgWidth;
            float scaleY = (float)SCREEN_HEIGHT / imgHeight;
            float scale = std::min(scaleX, scaleY) * 0.8f; // 稍微缩小一点，留出边距
            
            int scaledWidth = (int)(imgWidth * scale);
            int scaledHeight = (int)(imgHeight * scale);
            
            destRect = {
                (SCREEN_WIDTH - scaledWidth) / 2,
                (SCREEN_HEIGHT - scaledHeight) / 2,
                scaledWidth,
                scaledHeight
            };
            
            std::cout << "图片需要缩放，缩放比例: " << scale << std::endl;
            std::cout << "缩放后尺寸: " << scaledWidth << "x" << scaledHeight << std::endl;
        } else {
            // 图片比屏幕小，直接居中显示
            destRect = {
                (SCREEN_WIDTH - imgWidth) / 2,
                (SCREEN_HEIGHT - imgHeight) / 2,
                imgWidth,
                imgHeight
            };
            
            std::cout << "图片直接居中显示" << std::endl;
        }
        
        SDL_RenderCopy(renderer, deathImage, nullptr, &destRect);
    } else {
        // 备用：红色背景
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect screenRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderFillRect(renderer, &screenRect);
    }
    
    SDL_RenderPresent(renderer);
    SDL_Delay(16);
}

void Game::handlePlayerDeath() {
    std::cout << "=== 处理玩家死亡 ===" << std::endl;
    
    if (player && player->isDead()) {
        std::cout << "确认玩家死亡，显示死亡图片2秒" << std::endl;
        deathStartTime = SDL_GetTicks();
        gameState = STATE_DEATH_ANIMATION;
        std::cout << "游戏状态已切换到: STATE_DEATH_ANIMATION" << std::endl;
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

    try {
        map = new TiledMap("assets/maps/level1.tmj", renderer);
    } catch (const std::exception& e) {
        std::cerr << "地图加载失败: " << e.what() << std::endl;
        return;
    }

    // 关键修复：设置地图缩放为1.0f，简化坐标计算
    map->setRenderScale(1.0f);

    std::cout << "========= 地图详细信息 ==========" << std::endl;
    std::cout << "地图原始尺寸: " << map->getMapPixelWidth() << "x" << map->getMapPixelHeight() << std::endl;
    std::cout << "地图内容尺寸: " << map->getContentPixelWidth() << "x" << map->getContentPixelHeight() << std::endl;
    std::cout << "瓦片尺寸: " << map->getTileWidth() << "x" << map->getTileHeight() << std::endl;
    std::cout << "屏幕尺寸: " << SCREEN_WIDTH << "x" << SCREEN_HEIGHT << std::endl;
    std::cout << "=========================================================" << std::endl;

    // 关键修复：使用原始内容尺寸（因为缩放为1.0f）
    camera = new Camera(SCREEN_WIDTH, SCREEN_HEIGHT, 
                       map->getContentPixelWidth(), 
                       map->getContentPixelHeight());

    player = new Player(renderer);
    
    // 设置玩家在安全位置
    float startX = 100.0f;
    float startY = 100.0f;
    player->setPosition({startX, startY});

    // 加载金币贴图
    if (!coins.load(renderer, "assets/coin.png")) {
        SDL_Log("Failed to load assets/coin.png");
    }

    // 用固定坐标生成
    coins.spawnFixed({
        {200.f,140.f}, {360.f,220.f}, {520.f,340.f}
    }, 16); // 金币尺寸


    std::cout << "玩家初始位置: (" << startX << ", " << startY << ")" << std::endl;

    // 立即将相机对准玩家初始位置
    camera->follow(player->getPosition(), 1.0f);

    std::cout << "新游戏初始化完成" << std::endl;
    std::cout << "相机初始位置: (" << camera->x << ", " << camera->y << ")" << std::endl;
}