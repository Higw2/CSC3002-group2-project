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
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL初始化失败: " << SDL_GetError() << std::endl;
        return false;
    }

    // 创建窗口
    window = SDL_CreateWindow(
        "平台跳跃游戏",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        0
    );
    if (!window) {
        std::cerr << "窗口创建失败: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    // 创建渲染器
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "渲染器创建失败: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    // 设置默认背景色（深灰色）
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);

    // 初始化SDL_image
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image初始化失败: " << IMG_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    // 加载地图
    try {
        map = new TiledMap("assets/maps/level1.tmj", renderer);
    } catch (const std::exception& e) {
        std::cerr << "地图加载失败: " << e.what() << std::endl;
        return false;
    }

    // 设置地图缩放 - 根据新地图调整
    map->setRenderScale(1.0f);  // 改为1倍缩放

    // 输出详细地图信息
    std::cout << "=== 地图详细信息 ===" << std::endl;
    std::cout << "原始地图尺寸: " << map->getMapPixelWidth() << "x" << map->getMapPixelHeight() << std::endl;
    std::cout << "实际内容尺寸: " << map->getContentPixelWidth() << "x" << map->getContentPixelHeight() << std::endl;
    std::cout << "缩放后地图尺寸: " << map->getMapPixelWidth() * 2 << "x" << map->getMapPixelHeight() * 2 << std::endl;
    std::cout << "缩放后内容尺寸: " << map->getContentPixelWidth() * 2 << "x" << map->getContentPixelHeight() * 2 << std::endl;
    std::cout << "瓦片尺寸: " << map->getTileWidth() << "x" << map->getTileHeight() << std::endl;
    std::cout << "图像层数量: " << map->getImageLayerCount() << std::endl;
    std::cout << "屏幕尺寸: " << SCREEN_WIDTH << "x" << SCREEN_HEIGHT << std::endl;
    std::cout << "==================" << std::endl;

    // 初始化相机 - 使用实际内容尺寸
    int scaledContentWidth = map->getContentPixelWidth() * map->getRenderScale();
    int scaledContentHeight = map->getContentPixelHeight() * map->getRenderScale();
    camera = new Camera(SCREEN_WIDTH, SCREEN_HEIGHT, scaledContentWidth, scaledContentHeight);

    // 初始化玩家（放在地图左上角）
    player = new Player(renderer);
    
    // 设置玩家在左上角安全位置
    float startX = 100.0f;  // 离左边界100像素
    float startY = 100.0f;  // 离上边界100像素
    player->setPosition({startX, startY});

    std::cout << "玩家初始位置设置为左上角: (" << startX << ", " << startY << ")" << std::endl;

    // 立即将相机对准玩家初始位置
    camera->follow(player->getPosition(), map->getRenderScale());

    std::cout << "=== 摄像机初始设置 ===" << std::endl;
    std::cout << "地图内容尺寸: " << scaledContentWidth << "x" << scaledContentHeight << std::endl;
    std::cout << "玩家初始位置: (" << startX << ", " << startY << ")" << std::endl;
    std::cout << "摄像机初始位置: (" << camera->x << ", " << camera->y << ")" << std::endl;

    isRunning = true;
    return true;
}

void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            isRunning = false;
        }
    }
}

void Game::update() {
    player->handleInput();
    player->update(*map);
    
    glm::vec2 playerPos = player->getPosition();
    std::cout << "=== 相机跟随计算 ===" << std::endl;
    std::cout << "玩家原始位置: (" << playerPos.x << ", " << playerPos.y << ")" << std::endl;
    std::cout << "地图缩放: " << map->getRenderScale() << std::endl;
    
    camera->follow(playerPos, map->getRenderScale());
    
    std::cout << "==================" << std::endl;
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
        std::cerr << "初始化失败，退出程序" << std::endl;
        return;
    }

    while (isRunning) {
        handleEvents();
        update();
        render();
        SDL_Delay(16);  // 约60帧/秒
    }
}