#include "Coin.hpp"
#include "Camera.h"                // 需要访问 camera.x / camera.y
#include <SDL2/SDL_image.h>
#include <algorithm>
#include <cmath>

static inline bool Intersects(const SDL_Rect& a, const SDL_Rect& b) {
    return !(a.x + a.w <= b.x || b.x + b.w <= a.x ||
             a.y + a.h <= b.y || b.y + b.h <= a.y);
}

bool CoinManager::load(SDL_Renderer* renderer, const char* path) {
    coinTex_ = IMG_LoadTexture(renderer, path);
    return coinTex_ != nullptr;
}

void CoinManager::spawnFixed(const std::vector<SDL_FPoint>& pts, int size) {
    coins_.reserve(coins_.size() + pts.size());
    for (auto& p : pts) {
        coins_.push_back(Coin{
            SDL_Rect{ (int)std::lround(p.x), (int)std::lround(p.y), size, size },
            true
        });
    }
}

void CoinManager::updateOnPlayerCollision(const SDL_Rect& playerRect, int& score) {
    for (auto& c : coins_) {
        if (c.alive && Intersects(c.rect, playerRect)) {
            c.alive = false;
            ++score;
        }
    }
    coins_.erase(std::remove_if(coins_.begin(), coins_.end(),
        [](const Coin& c){ return !c.alive; }), coins_.end());
}

// 统一的签名：render(renderer, cam, renderScale)
void CoinManager::render(SDL_Renderer* renderer, const Camera& cam, float renderScale) const {
    if (!coinTex_) return;

    // 直接用公开成员 x/y（你的 Camera 没有 getPosition）
    const int camX = (int)std::lround(cam.x);
    const int camY = (int)std::lround(cam.y);

    for (auto& c : coins_) {
        if (!c.alive) continue;

        SDL_Rect dst;
        dst.x = (int)std::lround((c.rect.x - camX) * renderScale);
        dst.y = (int)std::lround((c.rect.y - camY) * renderScale);
        dst.w = (int)std::lround(c.rect.w * renderScale);
        dst.h = (int)std::lround(c.rect.h * renderScale);

        SDL_RenderCopy(renderer, coinTex_, nullptr, &dst);
    }
}

void CoinManager::clear() {
    coins_.clear();
    if (coinTex_) { SDL_DestroyTexture(coinTex_); coinTex_ = nullptr; }
}
