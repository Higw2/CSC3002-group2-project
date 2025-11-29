#include "Game.h"
#include <iostream>

Game::Game() {}

Game::~Game() {
    delete map;
    delete player;
    delete camera;
    if (startMenu) {
        delete startMenu;
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

bool Game::init() {
    // 初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) 
    {
        std::cerr << "SDL初始化失败: " << SDL_GetError() << std::endl;
        return false;
    }

    // 创建窗口
    window = SDL_CreateWindow(
        "EchoRidge",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        0
    );
    if (!window) {
        std::cerr << "fail to creat a window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    // 创建渲染器
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "fail to create a renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    // 设置默认背景色（深灰色）
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);

    // 初始化SDL_image
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "fail to init the SDL_image: " << IMG_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    // 初始化开始菜单
    startMenu = new StartMenu(renderer);
    if (!startMenu->init()) {
        std::cerr << "开始菜单初始化失败" << std::endl;
        // 不直接返回false，允许游戏继续运行
    }

    // 初始状态为菜单
    gameState = STATE_MENU;

    // 初始化时间变量
    lastUpdateTime = SDL_GetTicks();
    std::cout << "Time system initialized. LastUpdateTime: " << lastUpdateTime << std::endl;

    isRunning = true;
    return true;
}

void Game::handleEvents() 
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) 
    {
        if (event.type == SDL_QUIT) 
        {
            isRunning = false;
        }
        else if (event.type == SDL_KEYDOWN) {
            // 添加ESC键返回菜单的功能
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                if (gameState == STATE_PLAYING) {
                    gameState = STATE_MENU;
                    std::cout << "返回开始菜单" << std::endl;
                }
                // 注意：这里不处理菜单状态下的ESC，因为菜单有自己的ESC处理
            }
        }
    }
}

void Game::update(float deltaTime) {
    if (gameState != STATE_PLAYING) {
        return; // 只有游戏进行状态才更新游戏逻辑
    }

    player->handleInput();
    player->update(*map, deltaTime); // 传入时间增量
    
    glm::vec2 playerPos = player->getPosition();
    
    // 调试输出时间信息
    static int updateCount = 0;
    if (updateCount++ % 60 == 0) { // 每60帧输出一次
        std::cout << "=== Update Info ===" << std::endl;
        std::cout << "DeltaTime: " << deltaTime * 1000 << "ms" << std::endl;
        std::cout << "Player Position: (" << playerPos.x << ", " << playerPos.y << ")" << std::endl;
    }
    
    camera->follow(playerPos, map->getRenderScale());
}

void Game::render() {
    SDL_RenderClear(renderer);  // 清除为深灰色

    if (gameState == STATE_PLAYING) {
        // 游戏进行状态：渲染游戏场景
        map->renderBackground(renderer, *camera);  // 背景层（最底层）
        map->renderTiles(renderer, *camera);       // 瓦片层
        player->render(renderer, *camera, map->getRenderScale());  // 玩家（最上层）
    }
    // 菜单状态由菜单自己渲染

    SDL_RenderPresent(renderer);
}

void Game::run() {
    if (!init()) {
        std::cerr << "Initialization failed. Program terminated." << std::endl;
        return;
    }

    std::cout << "Game initialized successfully. Starting main loop..." << std::endl;

    while (isRunning) {
        // 计算时间增量
        Uint32 currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastUpdateTime) / 1000.0f; // 转换为秒
        lastUpdateTime = currentTime;

        // 限制最大时间增量，避免卡顿导致的物理异常
        if (deltaTime > MAX_DELTA_TIME) {
            std::cout << "Frame took too long: " << deltaTime * 1000 << "ms, clamping to " 
                      << MAX_DELTA_TIME * 1000 << "ms" << std::endl;
            deltaTime = MAX_DELTA_TIME;
        }

        // 输出帧率信息（可选，调试用）
        static Uint32 frameCount = 0;
        static Uint32 lastFpsTime = 0;
        frameCount++;
        if (currentTime - lastFpsTime >= 1000) { // 每秒输出一次
            float fps = frameCount / ((currentTime - lastFpsTime) / 1000.0f);
            std::cout << "FPS: " << fps << ", DeltaTime: " << deltaTime * 1000 << "ms" << std::endl;
            frameCount = 0;
            lastFpsTime = currentTime;
        }

        // 根据游戏状态执行不同的逻辑
        switch (gameState) {
            case STATE_MENU:
                runMenuState();
                break;
                
            case STATE_PLAYING:
                runPlayingState();
                break;
                
            case STATE_PAUSED:
                // 可以后续实现暂停菜单
                runPlayingState(); // 暂时使用游戏状态逻辑
                break;
        }
    }
}

void Game::runMenuState() {
    if (startMenu) {
        startMenu->reset(); // 重置菜单状态
        int choice = startMenu->run();
        
        // 处理菜单选择
        switch (choice) {
            case 0: // 开始游戏
                std::cout << "开始新游戏" << std::endl;
                startNewGame();
                gameState = STATE_PLAYING;
                break;
                
            case 1: // 继续游戏
                std::cout << "继续游戏" << std::endl;
                if (map && player && camera) {
                    gameState = STATE_PLAYING;
                } else {
                    startNewGame();
                    gameState = STATE_PLAYING;
                }
                break;
                
            case 2: // 设置
                std::cout << "打开设置" << std::endl;
                // 可以在这里添加设置菜单
                // 暂时不改变状态，继续显示主菜单
                break;
                
            case 3: // 退出游戏
                std::cout << "退出游戏" << std::endl;
                isRunning = false;
                break;
        }
    } else { 
        // 如果菜单初始化失败，直接开始游戏
        std::cout << "菜单不可用，直接开始游戏" << std::endl;
        startNewGame();
        gameState = STATE_PLAYING;
    }
}

void Game::runPlayingState() {
    handleEvents();
    update(deltaTime);
    render();

    // 帧率控制：目标60FPS（约16.67ms/帧）
    Uint32 frameTime = SDL_GetTicks() - lastUpdateTime;
    if (frameTime < 16) {
        SDL_Delay(16 - frameTime);
    }
}

void Game::startNewGame() {
    // 清理现有的游戏资源
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

    // 加载地图
    try {
        map = new TiledMap("assets/maps/level1.tmj", renderer);
    } catch (const std::exception& e) {
        std::cerr << "fail to load the maps: " << e.what() << std::endl;
        return;
    }

    // 设置地图缩放
    map->setRenderScale(1.0f);  // 改为1倍缩放

    // 输出详细地图信息
    std::cout << "========= the detail infomation of the maps ==========" << std::endl;
    std::cout << "the original size of the maps: " << map->getMapPixelWidth() << "x" << map->getMapPixelHeight() << std::endl;
    std::cout << "the actual size of the contant: " << map->getContentPixelWidth() << "x" << map->getContentPixelHeight() << std::endl;
    std::cout << "the size of the maps(after zooming): " << map->getMapPixelWidth() * 2 << "x" << map->getMapPixelHeight() * 2 << std::endl;
    std::cout << "the size of the contant(after zooming): " << map->getContentPixelWidth() * 2 << "x" << map->getContentPixelHeight() * 2 << std::endl;
    std::cout << "the size of the tiles: " << map->getTileWidth() << "x" << map->getTileHeight() << std::endl;
    std::cout << "the number of the image layer " << map->getImageLayerCount() << std::endl;
    std::cout << "the size of the screen" << SCREEN_WIDTH << "x" << SCREEN_HEIGHT << std::endl;
    std::cout << "=========================================================" << std::endl;

    // 初始化相机 - 使用实际内容尺寸
    int scaledContentWidth = map->getContentPixelWidth() * map->getRenderScale();
    int scaledContentHeight = map->getContentPixelHeight() * map->getRenderScale();
    camera = new Camera(SCREEN_WIDTH, SCREEN_HEIGHT, scaledContentWidth, scaledContentHeight);

    // 初始化玩家
    player = new Player(renderer);
    
    // 设置玩家在左上角安全位置
    float startX = 100.0f;  // 离左边界100像素
    float startY = 100.0f;  // 离上边界100像素
    player->setPosition({startX, startY});

    std::cout << "The player's initial position is set at the top left corner.: (" << startX << ", " << startY << ")" << std::endl;

    // 立即将相机对准玩家初始位置
    camera->follow(player->getPosition(), map->getRenderScale());

    std::cout << "=== Initial settings of the camera ===" << std::endl;
    std::cout << "Map content size: " << scaledContentWidth << "x" << scaledContentHeight << std::endl;
    std::cout << "Player's initial position: (" << startX << ", " << startY << ")" << std::endl;
    std::cout << "The initial position of the camera: (" << camera->x << ", " << camera->y << ")" << std::endl;

    std::cout << "新游戏初始化完成" << std::endl;
}