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

    void renderBackground(SDL_Renderer* renderer, const Camera& camera) const;  // 单独渲染背景
    void renderTiles(SDL_Renderer* renderer, const Camera& camera) const;       // 单独渲染瓦片
    bool isColliding(int worldX, int worldY) const;

    // 公共接口
    int getMapPixelWidth() const { return mapWidth * tileWidth; }
    int getMapPixelHeight() const { return mapHeight * tileHeight; }
    int getTileWidth() const { return tileWidth; }
    int getTileHeight() const { return tileHeight; }
    int getImageLayerCount() const { return imageLayers.size(); }

    // 缩放功能
    void setRenderScale(float scale) { renderScale = scale; }
    float getRenderScale() const { return renderScale; }

    // 实际内容尺寸
    int getContentPixelWidth() const { return contentPixelWidth; }
    int getContentPixelHeight() const { return contentPixelHeight; }

    // 获取图层数据用于调试
    const std::vector<std::vector<int>>& getMainLayer() const { return mainLayer; }
    const std::vector<std::vector<int>>& getBackLayer() const { return backLayer; }
    
    // 检查瓦片是否为固体
    bool isSolidTile(int tileId) const { return solidTiles.count(tileId) > 0; }

private:
    struct ImageLayer {
        SDL_Texture* texture = nullptr;
        int x = 0, y = 0;
        float opacity = 1.0f;
        float parallaxX = 1.0f;  // 视差系数
        bool repeatX = false;    // X轴重复
        int imageWidth = 0;
        int imageHeight = 0;
    };

    // 私有渲染函数
    void renderImageLayer(SDL_Renderer* renderer, const Camera& camera, const ImageLayer& layer) const;
    void renderBackLayer(SDL_Renderer* renderer, const Camera& camera) const;
    void renderMainLayer(SDL_Renderer* renderer, const Camera& camera) const;

    SDL_Texture* tileset = nullptr;
    int tileWidth = 16;         // 瓦片像素宽度
    int tileHeight = 16;        // 瓦片像素高度
    int mapWidth = 0;           // 横向瓦片数量
    int mapHeight = 0;          // 纵向瓦片数量
    int firstGid = 1;
    float renderScale = 1.0f;   // 渲染缩放系数
    int contentPixelWidth = 0;  // 实际内容宽度
    int contentPixelHeight = 0; // 实际内容高度

    std::unordered_set<int> solidTiles;
    std::vector<std::vector<int>> backLayer;
    std::vector<std::vector<int>> mainLayer;
    std::unordered_map<int, SDL_Texture*> itemTextures;
    std::unordered_map<int, std::pair<int, int>> itemSizes;
    std::vector<ImageLayer> imageLayers;  // 背景图像层
};