#include "TiledMap.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <SDL2/SDL_image.h>

TiledMap::TiledMap(const std::string& mapPath, SDL_Renderer* renderer) {
    std::cout << "开始加载地图: " << mapPath << std::endl;
    
    std::ifstream file(mapPath);
    if (!file.is_open()) {
        throw std::runtime_error("无法打开地图文件: " + mapPath);
    }

    json j;
    try {
        file >> j;
    } catch (const std::exception& e) {
        throw std::runtime_error("JSON解析失败: " + std::string(e.what()));
    }

    // 解析地图基本信息
    if (j.contains("tilewidth") && j.contains("tileheight") && 
        j.contains("width") && j.contains("height")) {
        tileWidth = j["tilewidth"].get<int>();
        tileHeight = j["tileheight"].get<int>();
        mapWidth = j["width"].get<int>();
        mapHeight = j["height"].get<int>();
    } else {
        throw std::runtime_error("地图文件缺少基本属性");
    }

    std::cout << "地图基本信息: " << mapWidth << "x" << mapHeight 
              << ", 瓦片: " << tileWidth << "x" << tileHeight << std::endl;

    // 解析瓦片集
    if (j.contains("tilesets")) {
        for (const auto& ts : j["tilesets"]) {
            if (!ts.contains("name") || !ts.contains("firstgid")) {
                std::cerr << "跳过无效的瓦片集: 缺少name或firstgid" << std::endl;
                continue;
            }

            std::string name = ts["name"].get<std::string>();
            int firstGid = ts["firstgid"].get<int>();

            std::cout << "处理瓦片集: " << name << " (firstgid: " << firstGid << ")" << std::endl;

            // 加载basic瓦片集
            if (name == "basic") {
                this->firstGid = firstGid;
                
                if (!ts.contains("image")) {
                    std::cerr << "basic瓦片集缺少image字段" << std::endl;
                    continue;
                }
                
                std::string imagePath = ts["image"].get<std::string>();
                std::string imgPath = "assets/" + imagePath;
                
                tileset = IMG_LoadTexture(renderer, imgPath.c_str());
                if (!tileset) {
                    std::cerr << "basic瓦片集加载失败: " << IMG_GetError() 
                              << ", 路径: " << imgPath << std::endl;
                    // 创建默认纹理作为后备
                    SDL_Surface* surface = SDL_CreateRGBSurface(0, 16, 16, 32, 0, 0, 0, 0);
                    SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, 255, 0, 0));
                    tileset = SDL_CreateTextureFromSurface(renderer, surface);
                    SDL_FreeSurface(surface);
                } else {
                    std::cout << "成功加载basic瓦片集: " << imgPath << std::endl;
                }

                // 解析碰撞属性
                if (ts.contains("tiles")) {
                    for (const auto& tile : ts["tiles"]) {
                        if (tile.contains("id") && tile.contains("properties")) {
                            int localId = tile["id"].get<int>();
                            int globalId = firstGid + localId;
                            for (const auto& prop : tile["properties"]) {
                                if (prop.contains("name") && prop.contains("value") && 
                                    prop["name"].get<std::string>() == "soild" && 
                                    prop["value"].get<bool>()) {
                                    solidTiles.insert(globalId);
                                }
                            }
                        }
                    }
                }
            }

            // 加载items瓦片集
            if (name == "items" && ts.contains("tiles")) {
                for (const auto& tile : ts["tiles"]) {
                    if (!tile.contains("id") || !tile.contains("image") ||
                        !tile.contains("imagewidth") || !tile.contains("imageheight")) {
                        std::cerr << "跳过无效的items瓦片: 缺少必需字段" << std::endl;
                        continue;
                    }
                    
                    int localId = tile["id"].get<int>();
                    int globalId = firstGid + localId;
                    std::string imagePath = tile["image"].get<std::string>();
                    std::string imgPath = "assets/" + imagePath;
                    
                    SDL_Texture* tex = IMG_LoadTexture(renderer, imgPath.c_str());
                    if (tex) {
                        itemTextures[globalId] = tex;
                        itemSizes[globalId] = {
                            tile["imagewidth"].get<int>(),
                            tile["imageheight"].get<int>()
                        };
                        std::cout << "成功加载items瓦片: " << imgPath << std::endl;
                    } else {
                        std::cerr << "items瓦片加载失败: " << IMG_GetError() 
                                  << ", 路径: " << imgPath << std::endl;
                    }
                }
            }
        }
    }

    // 解析图层
    if (j.contains("layers")) {
        for (const auto& layer : j["layers"]) {
            if (!layer.contains("name") || !layer.contains("type")) {
                std::cerr << "跳过无效的图层: 缺少name或type" << std::endl;
                continue;
            }

            std::string name = layer["name"].get<std::string>();
            std::string type = layer["type"].get<std::string>();

            std::cout << "处理图层: " << name << " (" << type << ")" << std::endl;

            // 解析图像层（背景）
            if (type == "imagelayer") {
                if (!layer.contains("image")) {
                    std::cerr << "图像层 " << name << " 缺少image字段" << std::endl;
                    continue;
                }

                ImageLayer imgLayer;
                imgLayer.x = layer.contains("x") ? layer["x"].get<int>() : 0;
                imgLayer.y = layer.contains("y") ? layer["y"].get<int>() : 0;
                imgLayer.opacity = layer.contains("opacity") ? layer["opacity"].get<float>() : 1.0f;
                imgLayer.parallaxX = layer.contains("parallaxx") ? layer["parallaxx"].get<float>() : 1.0f;
                imgLayer.repeatX = layer.contains("repeatx") ? layer["repeatx"].get<bool>() : false;
                
                if (layer.contains("imagewidth") && layer.contains("imageheight")) {
                    imgLayer.imageWidth = layer["imagewidth"].get<int>();
                    imgLayer.imageHeight = layer["imageheight"].get<int>();
                }

                std::string imgPath = "assets/" + layer["image"].get<std::string>();
                std::cout << "加载图像层: " << name << ", 路径: " << imgPath << std::endl;
                
                imgLayer.texture = IMG_LoadTexture(renderer, imgPath.c_str());
                if (imgLayer.texture) {
                    if (imgLayer.imageWidth == 0 || imgLayer.imageHeight == 0) {
                        SDL_QueryTexture(imgLayer.texture, nullptr, nullptr, 
                                       &imgLayer.imageWidth, &imgLayer.imageHeight);
                        std::cout << "从纹理获取尺寸: " << imgLayer.imageWidth << "x" << imgLayer.imageHeight << std::endl;
                    }
                    
                    imageLayers.push_back(imgLayer);
                    std::cout << "成功加载图像层: " << name 
                              << " (" << imgLayer.imageWidth << "x" << imgLayer.imageHeight << ")"
                              << ", 视差: " << imgLayer.parallaxX << std::endl;
                } else {
                    std::cerr << "图像层加载失败: " << name << " - " << IMG_GetError() 
                              << ", 路径: " << imgPath << std::endl;
                }
            }

            // 解析瓦片层
            if ((name == "back" || name == "main") && type == "tilelayer") {
                if (!layer.contains("data")) {
                    std::cerr << "瓦片层 " << name << " 缺少data字段" << std::endl;
                    continue;
                }

                const auto& data = layer["data"];
                
                // 获取图层尺寸（可能不同于地图尺寸）
                int layerWidth = layer.contains("width") ? layer["width"].get<int>() : mapWidth;
                int layerHeight = layer.contains("height") ? layer["height"].get<int>() : mapHeight;
                
                std::cout << "图层 " << name << " 尺寸: " << layerWidth << "x" << layerHeight << std::endl;
                
                if (name == "back") {
                    backLayer.resize(layerHeight, std::vector<int>(layerWidth, 0));
                    for (int y = 0; y < layerHeight; y++) {
                        for (int x = 0; x < layerWidth; x++) {
                            int index = y * layerWidth + x;
                            if (index < data.size()) {
                                backLayer[y][x] = data[index].get<int>();
                            }
                        }
                    }
                    std::cout << "back层加载完成，实际尺寸: " << backLayer.size() << "x" << (backLayer.empty() ? 0 : backLayer[0].size()) << std::endl;
                } else if (name == "main") {
                    mainLayer.resize(layerHeight, std::vector<int>(layerWidth, 0));
                    for (int y = 0; y < layerHeight; y++) {
                        for (int x = 0; x < layerWidth; x++) {
                            int index = y * layerWidth + x;
                            if (index < data.size()) {
                                mainLayer[y][x] = data[index].get<int>();
                            }
                        }
                    }
                    std::cout << "main层加载完成，实际尺寸: " << mainLayer.size() << "x" << (mainLayer.empty() ? 0 : mainLayer[0].size()) << std::endl;
                }
            }
        }
    }

    // 计算实际内容尺寸（基于main层）
    int maxContentX = 0;
    int maxContentY = 0;

    // 使用图层实际尺寸
    int layerWidth = mainLayer.empty() ? mapWidth : mainLayer[0].size();
    int layerHeight = mainLayer.empty() ? mapHeight : mainLayer.size();

    // 分析main层找到实际内容边界
    if (!mainLayer.empty()) {
        for (int y = 0; y < layerHeight; y++) {
            for (int x = 0; x < layerWidth; x++) {
                if (mainLayer[y][x] != 0) {
                    maxContentX = std::max(maxContentX, x);
                    maxContentY = std::max(maxContentY, y);
                }
            }
        }
    }

    // 如果没有找到内容，使用图层尺寸
    if (maxContentX == 0) maxContentX = layerWidth - 1;
    if (maxContentY == 0) maxContentY = layerHeight - 1;

    // 计算像素尺寸
    contentPixelWidth = (maxContentX + 1) * tileWidth;
    contentPixelHeight = (maxContentY + 1) * tileHeight;

    std::cout << "图层尺寸: " << layerWidth << "x" << layerHeight << " 瓦片" << std::endl;
    std::cout << "内容边界: X[0~" << maxContentX << "], Y[0~" << maxContentY << "] 瓦片" << std::endl;
    std::cout << "实际内容尺寸: " << contentPixelWidth << "x" << contentPixelHeight 
              << " (瓦片: " << (maxContentX + 1) << "x" << (maxContentY + 1) << ")" << std::endl;
    std::cout << "地图加载完成!" << std::endl;
    std::cout << "back层尺寸: " << (backLayer.empty() ? 0 : backLayer[0].size()) << "x" << backLayer.size() << std::endl;
    std::cout << "main层尺寸: " << (mainLayer.empty() ? 0 : mainLayer[0].size()) << "x" << mainLayer.size() << std::endl;
    std::cout << "固体瓦片数量: " << solidTiles.size() << std::endl;
    std::cout << "图像层数量: " << imageLayers.size() << std::endl;
}

TiledMap::~TiledMap() {
    SDL_DestroyTexture(tileset);
    for (auto& [id, tex] : itemTextures) SDL_DestroyTexture(tex);
    for (auto& layer : imageLayers) SDL_DestroyTexture(layer.texture);
}

// 渲染所有背景层
void TiledMap::renderBackground(SDL_Renderer* renderer, const Camera& camera) const {
    for (int i = 0; i < imageLayers.size(); i++) {
        const auto& layer = imageLayers[i];
        renderImageLayer(renderer, camera, layer);
    }
}

// 渲染单个背景层（应用视差和相机偏移）
void TiledMap::renderImageLayer(SDL_Renderer* renderer, const Camera& camera, const ImageLayer& layer) const {
    if (!layer.texture) {
        return;
    }

    const SDL_Rect& view = camera.getView();
    
    // 设置透明度
    if (layer.opacity < 1.0f) {
        SDL_SetTextureAlphaMod(layer.texture, (Uint8)(layer.opacity * 255));
    }

    // 计算背景层位置（考虑缩放）
    int screenX = (int)(layer.x * renderScale - view.x * layer.parallaxX);
    int screenY = (int)(layer.y * renderScale - view.y);

    // 计算内容边界（缩放后）
    int scaledContentWidth = contentPixelWidth * renderScale;
    int scaledContentHeight = contentPixelHeight * renderScale;

    // 处理X轴重复 - 但限制在内容宽度内
    if (layer.repeatX && layer.imageWidth > 0) {
        int scaledWidth = (int)(layer.imageWidth * renderScale);
        int startOffset = screenX % scaledWidth;
        if (startOffset > 0) startOffset -= scaledWidth;

        int totalWidth = 0;
        // 限制重复在内容宽度范围内
        while (totalWidth < std::min(view.w + scaledWidth, scaledContentWidth)) {
            SDL_Rect dest = {
                screenX + startOffset + totalWidth,
                screenY,
                scaledWidth,
                (int)(layer.imageHeight * renderScale)
            };
            SDL_RenderCopy(renderer, layer.texture, nullptr, &dest);
            totalWidth += scaledWidth;
        }
    } else {
        // 单次渲染，确保不超出内容范围
        SDL_Rect dest = {
            screenX, 
            screenY, 
            std::min((int)(layer.imageWidth * renderScale), scaledContentWidth), 
            std::min((int)(layer.imageHeight * renderScale), scaledContentHeight)
        };
        SDL_RenderCopy(renderer, layer.texture, nullptr, &dest);
    }

    // 恢复透明度设置
    SDL_SetTextureAlphaMod(layer.texture, 255);
}

// 渲染所有瓦片层（back + main）
void TiledMap::renderTiles(SDL_Renderer* renderer, const Camera& camera) const {
    renderBackLayer(renderer, camera);
    renderMainLayer(renderer, camera);
}

// 渲染back瓦片层（只渲染相机视野内）
void TiledMap::renderBackLayer(SDL_Renderer* renderer, const Camera& camera) const {
    if (backLayer.empty() || !tileset) return;

    const SDL_Rect& view = camera.getView();
    SDL_Rect srcRect = {0, 0, tileWidth, tileHeight};
    int tilesetWidth = 0;
    SDL_QueryTexture(tileset, nullptr, nullptr, &tilesetWidth, nullptr);
    int tilesPerRow = tilesetWidth / tileWidth;

    // 使用图层实际尺寸
    int layerWidth = backLayer[0].size();
    int layerHeight = backLayer.size();

    // 计算视野内的瓦片范围
    int startX = std::max(0, view.x / tileWidth);
    int startY = std::max(0, view.y / tileHeight);
    int endX = std::min(layerWidth, (view.x + view.w) / tileWidth + 1);
    int endY = std::min(layerHeight, (view.y + view.h) / tileHeight + 1);

    // 渲染可见瓦片
    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            int tileId = backLayer[y][x];
            if (tileId == 0) continue;

            // 计算屏幕坐标并应用缩放
            int screenX = (x * tileWidth - view.x) * renderScale;
            int screenY = (y * tileHeight - view.y) * renderScale;
            int scaledWidth = tileWidth * renderScale;
            int scaledHeight = tileHeight * renderScale;

            auto texIt = itemTextures.find(tileId);
            if (texIt != itemTextures.end()) {
                auto sizeIt = itemSizes.find(tileId);
                if (sizeIt != itemSizes.end()) {
                    auto [w, h] = sizeIt->second;
                    SDL_Rect dest = {
                        screenX, 
                        screenY - (h * renderScale - scaledHeight), 
                        w * renderScale, 
                        h * renderScale
                    };
                    SDL_RenderCopy(renderer, texIt->second, nullptr, &dest);
                }
            }
            // 渲染basic瓦片
            else {
                int localId = tileId - firstGid;
                srcRect.x = (localId % tilesPerRow) * tileWidth;
                srcRect.y = (localId / tilesPerRow) * tileHeight;
                SDL_Rect dest = {screenX, screenY, scaledWidth, scaledHeight};
                SDL_RenderCopy(renderer, tileset, &srcRect, &dest);
            }
        }
    }
}

// 渲染main瓦片层
void TiledMap::renderMainLayer(SDL_Renderer* renderer, const Camera& camera) const {
    if (mainLayer.empty() || !tileset) return;

    const SDL_Rect& view = camera.getView();
    SDL_Rect srcRect = {0, 0, tileWidth, tileHeight};
    int tilesetWidth = 0;
    SDL_QueryTexture(tileset, nullptr, nullptr, &tilesetWidth, nullptr);
    int tilesPerRow = tilesetWidth / tileWidth;

    // 使用图层实际尺寸
    int layerWidth = mainLayer[0].size();
    int layerHeight = mainLayer.size();

    int startX = std::max(0, view.x / tileWidth);
    int startY = std::max(0, view.y / tileHeight);
    int endX = std::min(layerWidth, (view.x + view.w) / tileWidth + 1);
    int endY = std::min(layerHeight, (view.y + view.h) / tileHeight + 1);

    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            int tileId = mainLayer[y][x];
            if (tileId == 0) continue;

            // 应用缩放
            int screenX = (x * tileWidth - view.x) * renderScale;
            int screenY = (y * tileHeight - view.y) * renderScale;
            int scaledWidth = tileWidth * renderScale;
            int scaledHeight = tileHeight * renderScale;
            
            int localId = tileId - firstGid;
            srcRect.x = (localId % tilesPerRow) * tileWidth;
            srcRect.y = (localId / tilesPerRow) * tileHeight;
            SDL_Rect dest = {screenX, screenY, scaledWidth, scaledHeight};
            SDL_RenderCopy(renderer, tileset, &srcRect, &dest);
        }
    }
}

bool TiledMap::isColliding(int worldX, int worldY) const {
    // 将世界坐标转换回原始地图坐标（考虑缩放）
    float originalX = (float)worldX / renderScale;
    float originalY = (float)worldY / renderScale;
    
    int tileX = (int)originalX / tileWidth;
    int tileY = (int)originalY / tileHeight;

    // 使用图层实际尺寸
    int layerWidth = mainLayer.empty() ? mapWidth : mainLayer[0].size();
    int layerHeight = mainLayer.empty() ? mapHeight : mainLayer.size();

    if (tileX < 0 || tileX >= layerWidth || tileY < 0 || tileY >= layerHeight) {
        return true;  // 图层外视为碰撞
    }

    int tileId = mainLayer[tileY][tileX];
    return solidTiles.count(tileId) > 0;
}