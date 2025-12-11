#include "Player.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>

Player::Player(SDL_Renderer* renderer) : position(0, 0), velocity(0, 0) {
    texture = IMG_LoadTexture(renderer, "assets/sprites/player.png");
    if (!texture) {
        std::cerr << "玩家纹理加载失败，使用红色方块代替: " << IMG_GetError() << std::endl;
        SDL_Surface* surface = SDL_CreateRGBSurface(0, 16, 24, 32, 0, 0, 0, 0);
        SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, 255, 0, 0));
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }

    hitbox = {2, 4, 12, 18};
    wasOnGround = true;
}

Player::~Player() {
    SDL_DestroyTexture(texture);
}

void Player::setAudioCallback(std::function<void(const std::string&)> callback) {
    audioCallback = callback;
}

void Player::playSound(const std::string& soundName) {
    if (audioCallback) {
        audioCallback(soundName);
    }
}

void Player::handleInput() {
    if (dead) {
        return;
    }

    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    velocity.x = 0;

    if (keys[SDL_SCANCODE_LEFT]) {
        velocity.x = -speed;
    }
    if (keys[SDL_SCANCODE_RIGHT]) {
        velocity.x = speed;
    }

    if ((keys[SDL_SCANCODE_SPACE] || keys[SDL_SCANCODE_UP]) && onGround) {
        velocity.y = jumpForce;
        onGround = false;
        playSound("jump");
        std::cout << "玩家起跳，播放 jump 音效" << std::endl;
    }
}

void Player::update(const TiledMap& map, float deltaTime) {
    if (dead) {
        return;
    }

    bool wasOnGroundBeforeUpdate = onGround;

    velocity.y += gravity * deltaTime;

    float oldX = position.x;
    position.x += velocity.x * deltaTime;

    SDL_Point testPoints[4] = {
        {static_cast<int>(position.x + hitbox.x), static_cast<int>(position.y + hitbox.y)},
        {static_cast<int>(position.x + hitbox.x + hitbox.w - 1),
         static_cast<int>(position.y + hitbox.y)},
        {static_cast<int>(position.x + hitbox.x), static_cast<int>(position.y + hitbox.y + hitbox.h - 1)},
        {static_cast<int>(position.x + hitbox.x + hitbox.w - 1),
         static_cast<int>(position.y + hitbox.y + hitbox.h - 1)}};

    bool collideX = false;
    for (int i = 0; i < 4; i++) {
        if (map.isColliding(testPoints[i].x, testPoints[i].y)) {
            collideX = true;
            break;
        }
    }

    if (collideX) {
        position.x = oldX;
        velocity.x = 0;
    }

    float oldY = position.y;
    position.y += velocity.y * deltaTime;

    SDL_Point testPointsY[4] = {
        {static_cast<int>(position.x + hitbox.x), static_cast<int>(position.y + hitbox.y)},
        {static_cast<int>(position.x + hitbox.x + hitbox.w - 1),
         static_cast<int>(position.y + hitbox.y)},
        {static_cast<int>(position.x + hitbox.x), static_cast<int>(position.y + hitbox.y + hitbox.h - 1)},
        {static_cast<int>(position.x + hitbox.x + hitbox.w - 1),
         static_cast<int>(position.y + hitbox.y + hitbox.h - 1)}};

    bool collideY = false;
    bool collideTop = false;
    bool collideBottom = false;

    for (int i = 0; i < 4; i++) {
        if (map.isColliding(testPointsY[i].x, testPointsY[i].y)) {
            collideY = true;
            if (i < 2) {
                collideTop = true;
            } else {
                collideBottom = true;
            }
        }
    }

    if (collideY) {
        if (collideBottom && velocity.y > 0) {
            onGround = true;
            float worldY = position.y + hitbox.y + hitbox.h;
            int tileY = static_cast<int>(worldY) / map.getTileHeight();
            position.y = (tileY * map.getTileHeight()) - hitbox.y - hitbox.h;
            velocity.y = 0;
        } else if (collideTop && velocity.y < 0) {
            position.y = oldY;
            velocity.y = 0;
        }
    } else {
        onGround = false;
    }

    if (!wasOnGroundBeforeUpdate && onGround && velocity.y >= 0.1f) {
        static Uint32 lastHurtTime = 0;
        Uint32 currentTime = SDL_GetTicks();
        // 300ms 冷却，避免落地音效过于频繁
        if (currentTime - lastHurtTime > 300) {
            playSound("hurt");
            std::cout << "播放落地音效 (速度: " << velocity.y << ")" << std::endl;
            lastHurtTime = currentTime;
        }
    }

    float contentWidth = map.getContentPixelWidth();
    float contentHeight = map.getContentPixelHeight();

    if (position.x < 0) {
        position.x = 0;
    }
    if (position.x + 16 > contentWidth) {
        position.x = contentWidth - 16;
    }
    if (position.y < 0) {
        position.y = 0;
    }
    if (position.y + 24 > contentHeight) {
        position.y = contentHeight - 24;
        velocity.y = 0;
        onGround = true;
    }

    checkCollisionsWithHazards(map);
}

void Player::checkCollisionsWithHazards(const TiledMap& map) {
    if (dead) {
        return;
    }

    SDL_Point testPoints[4] = {
        {static_cast<int>(position.x + hitbox.x), static_cast<int>(position.y + hitbox.y)},
        {static_cast<int>(position.x + hitbox.x + hitbox.w - 1),
         static_cast<int>(position.y + hitbox.y)},
        {static_cast<int>(position.x + hitbox.x), static_cast<int>(position.y + hitbox.y + hitbox.h - 1)},
        {static_cast<int>(position.x + hitbox.x + hitbox.w - 1),
         static_cast<int>(position.y + hitbox.y + hitbox.h - 1)}};

    for (int i = 0; i < 4; i++) {
        if (map.isHazard(testPoints[i].x, testPoints[i].y)) {
            std::cout << "玩家死亡，危险碰撞点 " << i << " ("
                      << testPoints[i].x << ", " << testPoints[i].y << ")" << std::endl;
            kill();
            return;
        }
    }
}

void Player::render(SDL_Renderer* renderer, const Camera& camera, float renderScale) {
    if (dead) {
        SDL_SetTextureAlphaMod(texture, 128);
    }

    const SDL_Rect& view = camera.getView();

    SDL_Rect dest = {static_cast<int>(position.x * renderScale - view.x),
                     static_cast<int>(position.y * renderScale - view.y),
                     static_cast<int>(16 * renderScale),
                     static_cast<int>(24 * renderScale)};

    SDL_RenderCopy(renderer, texture, nullptr, &dest);

    if (dead) {
        SDL_SetTextureAlphaMod(texture, 255);
    }
}
