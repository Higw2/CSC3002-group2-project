#pragma once
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include "TiledMap.h"

class Player {
public:
    Player(SDL_Renderer* renderer);
    ~Player();
    void handleInput();
    void update(const TiledMap& map);
    void render(SDL_Renderer* renderer, const Camera& camera, float renderScale = 1.0f);
    glm::vec2 getPosition() const { return position; }
    void setPosition(const glm::vec2& pos) { position = pos; }

private:
    SDL_Texture* texture = nullptr;
    glm::vec2 position;    // 玩家坐标
    glm::vec2 velocity;    // 速度（像素/帧）
    SDL_Rect hitbox;       // 碰撞盒（相对位置）
    bool onGround = false; // 是否在地面上

    // 移动参数
    const float speed = 3.0f;    // 水平速度
    const float jumpForce = -8.0f; // 跳跃初速度（负为上）
    const float gravity = 0.6f;  // 重力加速度
};