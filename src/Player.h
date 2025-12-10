#pragma once
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <functional>  // 添加这行
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
 
    SDL_Rect getWorldRect() const {
        return SDL_Rect{
            static_cast<int>(position.x + hitbox.x),
            static_cast<int>(position.y + hitbox.y),
            hitbox.w, hitbox.h
        };
    }

    // 修改：使用 std::function
    void setAudioCallback(std::function<void(const std::string&)> callback);
    
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
    bool dead = false;
    bool wasOnGround = true;
    bool justLanded = false;  // 新增：刚落地标志

    const float speed = 180.0f;
    const float jumpForce = -480.0f;
    const float gravity = 1200.0f;

    // 修改：使用 std::function
    std::function<void(const std::string&)> audioCallback = nullptr;
    void playSound(const std::string& soundName);
};