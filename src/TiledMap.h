#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "nlohmann/json.hpp"
#include "Camera.h"
using json = nlohmann::json;

class TiledMap {
public:
    TiledMap(const std::string& mapPath, SDL_Renderer* renderer);
    ~TiledMap();
    void renderBackground(SDL_Renderer* renderer, const Camera& camera) const;
    void renderTiles(SDL_Renderer* renderer, const Camera& camera) const;
    bool isColliding(int worldX, int worldY) const;

    // 公共接口
    int getMapPixelWidth() const { return mapWidth * tileWidth; }
    int getMapPixelHeight() const { return mapHeight * tileHeight; }
    int getTileWidth() const { return tileWidth; }
    int getTileHeight() const { return tileHeight; }
    int getImageLayerCount() const { return imageLayers.size(); }
    void setRenderScale(float scale) { renderScale = scale; }
    float getRenderScale() const { return renderScale; }
    int getContentPixelWidth() const { return contentPixelWidth; }
    int getContentPixelHeight() const { return contentPixelHeight; }
    const std::vector<std::vector<int>>& getMainLayer() const { return mainLayer; }
    const std::vector<std::vector<int>>& getBackLayer() const { return backLayer; }
    bool isSolidTile(int tileId) const { return solidTiles.count(tileId) > 0; }

private:
    struct ImageLayer {
        SDL_Texture* texture = nullptr;
        int x = 0, y = 0;
        float opacity = 1.0f;
        float parallaxX = 1.0f;
        bool repeatX = false;
        int imageWidth = 0;
        int imageHeight = 0;
        int offsetx = 0;
        int offsety = 0;
    };

    // 私有渲染函数
    void renderImageLayer(SDL_Renderer* renderer, const Camera& camera, const ImageLayer& layer) const;
    void renderBackLayer(SDL_Renderer* renderer, const Camera& camera) const;
    void renderMainLayer(SDL_Renderer* renderer, const Camera& camera) const;

    // 成员变量（关键：将 tilesetMap/firstGidMap 改为类成员，而非局部变量）
    int tileWidth = 16;
    int tileHeight = 16;
    int mapWidth = 0;
    int mapHeight = 0;
    float renderScale = 1.0f;
    int contentPixelWidth = 0;
    int contentPixelHeight = 0;
    std::unordered_set<int> solidTiles;
    std::vector<std::vector<int>> backLayer;
    std::vector<std::vector<int>> mainLayer;
    std::unordered_map<int, SDL_Texture*> itemTextures;
    std::unordered_map<int, std::pair<int, int>> itemSizes;
    std::vector<ImageLayer> imageLayers;
    // 关键修复：瓦片集映射改为类成员，确保渲染时可访问
    std::unordered_map<std::string, SDL_Texture*> tilesetMap;  
    std::unordered_map<std::string, int> firstGidMap;          
};