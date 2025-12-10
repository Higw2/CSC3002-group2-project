#include "Coin.hpp"
#include "Camera.h"
#include "TiledMap.h"
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

// 修改：返回bool表示是否有金币被收集
bool CoinManager::updateOnPlayerCollision(const SDL_Rect& playerRect, TiledMap& map, int& score) {
    bool coinCollected = false;  // 新增：追踪是否有金币被收集
    
    for (auto& c : coins_) {
        if (c.alive && Intersects(c.rect, playerRect)) {
            c.alive = false;
            ++score;
            coinCollected = true;  // 标记有金币被收集
            map.clearCoinTileAt(c.rect.x, c.rect.y);
        }
    }
    
    // 移除已收集的金币
    coins_.erase(std::remove_if(coins_.begin(), coins_.end(),
        [](const Coin& c){ return !c.alive; }), coins_.end());
    
    return coinCollected;  // 返回收集状态
}

void CoinManager::render(SDL_Renderer* renderer, const Camera& cam, float renderScale) const {
    if (!coinTex_) return;

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