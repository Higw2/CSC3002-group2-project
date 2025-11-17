#include "Game.h"
#include <iostream>

Game::Game() {}

Game::~Game() {
    delete map;
    delete player;
    delete camera;
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

    // 加载地图
    try {
        map = new TiledMap("assets/maps/level1.tmj", renderer);
    } catch (const std::exception& e) {
        std::cerr << "fail to load the maps: " << e.what() << std::endl;
        return false;
    }

    // 设置地图缩放 - 根据新地图调整
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

    // 初始化时间变量 - 在所有初始化完成后设置
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
    }
}

void Game::update(float deltaTime) {
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

    // 按顺序渲染（背景→瓦片→玩家）
    map->renderBackground(renderer, *camera);  // 背景层（最底层）
    map->renderTiles(renderer, *camera);       // 瓦片层
    player->render(renderer, *camera, map->getRenderScale());  // 玩家（最上层）

    SDL_RenderPresent(renderer);
}

void Game::run() {
    if (!init()) {
        std::cerr << "Initialization failed. Program terminated." << std::endl;
        return;
    }

    std::cout << "Starting time-based game loop..." << std::endl;

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

        handleEvents();
        update(deltaTime);  // 传入时间增量
        render();

        // 帧率控制：目标60FPS（约16.67ms/帧）
        Uint32 frameTime = SDL_GetTicks() - currentTime;
        if (frameTime < 16) {
            SDL_Delay(16 - frameTime);
        }
    }
}
