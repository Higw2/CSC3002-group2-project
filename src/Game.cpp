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

    // 初始化相机 - 使用实际内容尺寸(要不要使用地图尺寸，用内容尺寸容易出问题，到时候改一下)！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！
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

void Game::update() {
    player->handleInput();
    player->update(*map);
    
    glm::vec2 playerPos = player->getPosition();
    std::cout << "=== Camera tracking calculation ===" << std::endl;
    std::cout << "Player's original position: (" << playerPos.x << ", " << playerPos.y << ")" << std::endl;
    std::cout << "Map zooming: " << map->getRenderScale() << std::endl;
    
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
        std::cerr << "Initialization failed. Program terminated." << std::endl;
        return;
    }

    while (isRunning) {
        handleEvents();
        update();
        render();
        SDL_Delay(16);  // 约60帧/秒
    }
}