#include "Player.h"
#include <SDL2/SDL_image.h>
#include <iostream>

Player::Player(SDL_Renderer* renderer) : position(0, 0), velocity(0, 0) {
    // 加载玩家纹理
    texture = IMG_LoadTexture(renderer, "assets/sprites/player.png");
    if (!texture) {
        std::cerr << "The player texture failed to load. Using the default red rectangle instead.: " << IMG_GetError() << std::endl;
        // 创建默认红色纹理
        SDL_Surface* surface = SDL_CreateRGBSurface(0, 16, 24, 32, 0, 0, 0, 0);
        SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, 255, 0, 0));
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }

    // 碰撞盒（相对玩家左上角）
    hitbox = {2, 4, 12, 18};  // 微调碰撞范围
}

Player::~Player() {
    SDL_DestroyTexture(texture);
}

void Player::handleInput() {
    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    velocity.x = 0;  // 重置水平速度

    // 左右移动 - 速度值现在在update中与deltaTime结合
    if (keys[SDL_SCANCODE_LEFT])  velocity.x = -speed;
    if (keys[SDL_SCANCODE_RIGHT]) velocity.x = speed;

    // 跳跃（仅在地面）
    if ((keys[SDL_SCANCODE_SPACE] || keys[SDL_SCANCODE_UP]) && onGround) {
        velocity.y = jumpForce;
        onGround = false;
        std::cout << "Jump! Velocity: " << velocity.y << " pixels/sec" << std::endl;
    }
}

// 修改：添加deltaTime参数
void Player::update(const TiledMap& map, float deltaTime) {
    // 应用重力（基于时间）
    velocity.y += gravity * deltaTime;

    // 调试输出物理信息
    static int physicsLogCount = 0;
    if (physicsLogCount++ % 30 == 0) {
        std::cout << "Physics Update - DeltaTime: " << deltaTime 
                  << ", Gravity applied: " << gravity * deltaTime
                  << ", Velocity: (" << velocity.x << ", " << velocity.y << ")" << std::endl;
    }

    // X方向移动与碰撞
    float oldX = position.x;
    position.x += velocity.x * deltaTime;
    
    // 碰撞检测（使用修复后的版本 - 无缩放坐标）
    SDL_Point testPoints[4] = {
        {(int)(position.x + hitbox.x), (int)(position.y + hitbox.y)},
        {(int)(position.x + hitbox.x + hitbox.w - 1), (int)(position.y + hitbox.y)},
        {(int)(position.x + hitbox.x), (int)(position.y + hitbox.y + hitbox.h - 1)},
        {(int)(position.x + hitbox.x + hitbox.w - 1), (int)(position.y + hitbox.y + hitbox.h - 1)}
    };
    
    bool collideX = false;
    for (int i = 0; i < 4; i++) {
        if (map.isColliding(testPoints[i].x, testPoints[i].y)) {
            collideX = true;
            break;
        }
    }
    
    if (collideX) {
        position.x = oldX;  // 回退X位置
        velocity.x = 0;
    }

    // Y方向移动与碰撞
    float oldY = position.y;
    position.y += velocity.y * deltaTime;
    
    // 重新计算测试点（Y方向移动后）
    SDL_Point testPointsY[4] = {
        {(int)(position.x + hitbox.x), (int)(position.y + hitbox.y)},
        {(int)(position.x + hitbox.x + hitbox.w - 1), (int)(position.y + hitbox.y)},
        {(int)(position.x + hitbox.x), (int)(position.y + hitbox.y + hitbox.h - 1)},
        {(int)(position.x + hitbox.x + hitbox.w - 1), (int)(position.y + hitbox.y + hitbox.h - 1)}
    };
    
    bool collideY = false;
    bool collideTop = false;
    bool collideBottom = false;
    
    for (int i = 0; i < 4; i++) {
        if (map.isColliding(testPointsY[i].x, testPointsY[i].y)) {
            collideY = true;
            if (i < 2) collideTop = true;    // 前两个点是顶部
            else collideBottom = true;       // 后两个点是底部
        }
    }

    if (collideY) {
        if (collideBottom && velocity.y > 0) {  // 下落时碰撞（地面）
            onGround = true;
            // 对齐到瓦片顶部
            float worldY = position.y + hitbox.y + hitbox.h;
            int tileY = (int)(worldY) / map.getTileHeight();
            position.y = (tileY * map.getTileHeight()) - hitbox.y - hitbox.h;
            velocity.y = 0;
            std::cout << "Landed on ground" << std::endl;
        } 
        else if (collideTop && velocity.y < 0) {  // 上升时碰撞（天花板）
            position.y = oldY;
            velocity.y = 0;
            std::cout << "Hit ceiling" << std::endl;
        }
    } 
    else 
    {
        onGround = false;
    }

    // 边界检查
    float contentWidth = map.getContentPixelWidth();
    float contentHeight = map.getContentPixelHeight();
    
    if (position.x < 0) position.x = 0;
    if (position.x + 16 > contentWidth) {
        position.x = contentWidth - 16;
    }
    if (position.y < 0) position.y = 0;
    if (position.y + 24 > contentHeight) {
        position.y = contentHeight - 24;
        velocity.y = 0;
        onGround = true;
    }

    // 简化调试输出
    static int outputCount = 0;
    if (outputCount++ % 60 == 0) {
        std::cout << "Player - Pos: (" << position.x << ", " << position.y 
                  << ") Vel: (" << velocity.x << ", " << velocity.y 
                  << ") OnGround: " << onGround << std::endl;
    }
}

void Player::render(SDL_Renderer* renderer, const Camera& camera, float renderScale) {
    const SDL_Rect& view = camera.getView();
    
    // 应用缩放
    SDL_Rect dest = {
        (int)((position.x - view.x) * renderScale),
        (int)((position.y - view.y) * renderScale),
        (int)(16 * renderScale),
        (int)(24 * renderScale)
    };
    SDL_RenderCopy(renderer, texture, nullptr, &dest);
}

