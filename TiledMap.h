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
    bool isHazard(int worldX, int worldY) const;  // 新增：危险物检测

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
    bool isHazardTile(int tileId) const { return hazardTiles.count(tileId) > 0; }
    bool isCoinTile(int gid) const {
        return coinFirstGid >= 0 && gid >= coinFirstGid && gid < coinFirstGid + coinTileCount;
    }
    std::vector<SDL_FPoint> getCoinPositions() const;
    bool clearCoinTileAt(int worldX, int worldY);

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

    void renderImageLayer(SDL_Renderer* renderer, const Camera& camera, const ImageLayer& layer) const;
    void renderBackLayer(SDL_Renderer* renderer, const Camera& camera) const;
    void renderMainLayer(SDL_Renderer* renderer, const Camera& camera) const;
    void markTilesAsHazards();  // 新增：临时标记危险瓦片

    int tileWidth = 16;
    int tileHeight = 16;
    int mapWidth = 0;
    int mapHeight = 0;
    float renderScale = 1.0f;
    int contentPixelWidth = 0;
    int contentPixelHeight = 0;
    std::unordered_set<int> solidTiles;
    std::unordered_set<int> hazardTiles;  // 危险瓦片集合
    std::vector<std::vector<int>> backLayer;
    std::vector<std::vector<int>> mainLayer;
    std::unordered_map<int, SDL_Texture*> itemTextures;
    std::unordered_map<int, std::pair<int, int>> itemSizes;
    std::vector<ImageLayer> imageLayers;
    std::unordered_map<std::string, SDL_Texture*> tilesetMap;
    std::unordered_map<std::string, int> firstGidMap;
    int coinFirstGid = -1;
    int coinTileCount = 0;
};
