#pragma once
#include <SDL2/SDL.h>
#include <vector>

class Camera;
class TiledMap;

struct Coin {
    SDL_Rect rect;
    bool alive = true;
};

class CoinManager {
public:
    bool load(SDL_Renderer* renderer, const char* path);
    void spawnFixed(const std::vector<SDL_FPoint>& pts, int size);
    bool updateOnPlayerCollision(const SDL_Rect& playerRect, TiledMap& map, int& score); // 修改：返回bool
    void render(SDL_Renderer* renderer, const Camera& cam, float renderScale) const;
    void clear();
private:
    std::vector<Coin> coins_;
    SDL_Texture* coinTex_ = nullptr;
};