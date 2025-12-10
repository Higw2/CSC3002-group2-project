#pragma once
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include "TiledMap.h"
#include "Camera.h"

class Player {
public:
    Player(SDL_Renderer* renderer);
    ~Player();
    void handleInput();
    void update(const TiledMap& map, float deltaTime);
    void render(SDL_Renderer* renderer, const Camera& camera, float renderScale = 1.0f);
    glm::vec2 getPosition() const { return position; }
    void setPosition(const glm::vec2& pos) { position = pos; }
 
    //把玩家的世界坐标包围盒给金币系统做相交判断
    SDL_Rect getWorldRect() const {
        return SDL_Rect{
            static_cast<int>(position.x + hitbox.x),
            static_cast<int>(position.y + hitbox.y),
            hitbox.w, hitbox.h
        };
    }


    
    // 死亡判定相关方法
    bool isDead() const { return dead; }
    void kill() { dead = true; }
    void respawn() { dead = false; }
    void checkCollisionsWithHazards(const TiledMap& map);

private:
    SDL_Texture* texture = nullptr;
    glm::vec2 position;
    glm::vec2 velocity;
    SDL_Rect hitbox;
    bool onGround = false;
    bool dead = false;  // 死亡状态

    // 移动参数
    const float speed = 180.0f;
    const float jumpForce = -480.0f;
    const float gravity = 1200.0f;
};