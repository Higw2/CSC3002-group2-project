#include "TiledMap.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <SDL2/SDL_image.h>

TiledMap::TiledMap(const std::string& mapPath, SDL_Renderer* renderer) 
{
    std::cout << "Start loading map: " << mapPath << std::endl;
    
    std::ifstream file(mapPath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open map file: " + mapPath);
    }
    json j;
    try {
        file >> j;
    } 
    catch (const std::exception& e) 
    {
        throw std::runtime_error("JSON parsing failed: " + std::string(e.what()));
    }

    // 1. 解析地图基本信息
    if (!j.contains("tilewidth") || !j["tilewidth"].is_number_integer() ||
        !j.contains("tileheight") || !j["tileheight"].is_number_integer() ||
        !j.contains("width") || !j["width"].is_number_integer() ||
        !j.contains("height") || !j["height"].is_number_integer()) {
        throw std::runtime_error("Map file missing required properties (tilewidth/tileheight/width/height) or invalid type");
    }
    tileWidth = j["tilewidth"].get<int>();
    tileHeight = j["tileheight"].get<int>();
    mapWidth = j["width"].get<int>();
    mapHeight = j["height"].get<int>();
    std::cout << "Map basic info: " << mapWidth << "x" << mapHeight 
              << ", tiles: " << tileWidth << "x" << tileHeight << std::endl;

    // 2. 解析瓦片集
    if (j.contains("tilesets") && j["tilesets"].is_array()) 
    {
        for (const auto& ts : j["tilesets"]) 
        {
            if (!ts.contains("name") || !ts["name"].is_string() ||
                !ts.contains("firstgid") || !ts["firstgid"].is_number_integer()) 
            {
                std::cerr << "Skipping invalid tileset: Missing 'name' (string) or 'firstgid' (integer)" << std::endl;
                continue;
            }
            std::string name = ts["name"].get<std::string>();
            int firstGid = ts["firstgid"].get<int>();
            firstGidMap[name] = firstGid;
            std::cout << "Processing tileset: " << name << " (firstgid: " << firstGid << ")" << std::endl;

            // 3. 加载瓦片集纹理 - 修改：加载所有瓦片集纹理，包括items
            SDL_Texture* tilesetTex = nullptr;
            if (ts.contains("image") && ts["image"].is_string()) {
                std::string imagePath = ts["image"].get<std::string>();
                std::string imgPath = "assets/" + imagePath;
                tilesetTex = IMG_LoadTexture(renderer, imgPath.c_str());
                if (!tilesetTex) {
                    std::cerr << "Failed to load tileset texture: " << name << " - " << IMG_GetError() 
                              << ", path: " << imgPath << std::endl;
                    // 创建默认纹理
                    SDL_Surface* surface = SDL_CreateRGBSurface(0, tileWidth, tileHeight, 32, 0, 0, 0, 0);
                    SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, 128, 128, 128));
                    tilesetTex = SDL_CreateTextureFromSurface(renderer, surface);
                    SDL_FreeSurface(surface);
                } else {
                    std::cout << "Successfully loaded tileset: " << name << " (" << imgPath << ")" << std::endl;
                }
            } else {
                // 对于没有整体图片的瓦片集（如items），创建默认纹理
                SDL_Surface* surface = SDL_CreateRGBSurface(0, tileWidth, tileHeight, 32, 0, 0, 0, 0);
                SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, 128, 128, 128));
                tilesetTex = SDL_CreateTextureFromSurface(renderer, surface);
                SDL_FreeSurface(surface);
                std::cout << "Created default texture for tileset: " << name << std::endl;
            }
            tilesetMap[name] = tilesetTex;

            // 4. 解析瓦片属性（包括碰撞属性和危险属性）
            if (ts.contains("tiles") && ts["tiles"].is_array()) 
            {
                for (const auto& tile : ts["tiles"]) 
                {
                    if (!tile.is_object() || !tile.contains("id") || !tile["id"].is_number_integer()) {
                        std::cerr << "Skipping invalid tile in " << name << ": Not an object or missing 'id' (integer)" << std::endl;
                        continue;
                    }
                    int localId = tile["id"].get<int>();
                    int globalId = firstGid + localId;

                    // 处理碰撞属性和危险属性
                    if (tile.contains("properties") && tile["properties"].is_array()) {
                        for (const auto& prop : tile["properties"]) {
                            if (!prop.is_object() || !prop.contains("name") || !prop["name"].is_string()) {
                                continue;
                            }
                            std::string propName = prop["name"].get<std::string>();
                            
                            // 处理碰撞属性
                            if (propName == "Flat_Terrain" && prop.contains("value") && prop["value"].is_boolean()) {
                                bool propValue = prop["value"].get<bool>();
                                if (propValue) {
                                    solidTiles.insert(globalId);
                                    std::cout << "  - Tile " << globalId << " (local " << localId << ") marked as solid" << std::endl;
                                }
                            }
                            // 处理尖刺危险属性 - 新增
                            else if (propName == "Spike" && prop.contains("value") && prop["value"].is_boolean()) {
                                bool propValue = prop["value"].get<bool>();
                                if (propValue) {
                                    hazardTiles.insert(globalId);
                                    std::cout << "  - Tile " << globalId << " (local " << localId << ") marked as HAZARD (Spike)" << std::endl;
                                }
                            }
                        }
                    }

                    // items瓦片集处理（保持不变）
                    if (name == "items") {
                        if (!tile.contains("image") || !tile["image"].is_string() ||
                            !tile.contains("imagewidth") || !tile["imagewidth"].is_number_integer() ||
                            !tile.contains("imageheight") || !tile["imageheight"].is_number_integer()) {
                            std::cerr << "Skipping invalid items tile " << localId << ": Missing 'image' (string) or size (integer)" << std::endl;
                            continue;
                        }

                        std::string imagePath = tile["image"].get<std::string>();
                        std::string imgPath = "assets/" + imagePath;
                        int imgW = tile["imagewidth"].get<int>();
                        int imgH = tile["imageheight"].get<int>();

                        SDL_Texture* itemTex = IMG_LoadTexture(renderer, imgPath.c_str());
                        if (itemTex) {
                            itemTextures[globalId] = itemTex;
                            itemSizes[globalId] = {imgW, imgH};
                            std::cout << "  - Loaded items tile " << globalId << ": " << imgPath << " (" << imgW << "x" << imgH << ")" << std::endl;
                        } else {
                            std::cerr << "  - Failed to load items tile " << localId << ": " << IMG_GetError() << ", path: " << imgPath << std::endl;
                        }
                    }
                }
            }
        }
    }

    // 5. 解析图层（保持不变）
    if (j.contains("layers") && j["layers"].is_array()) 
    {
        for (const auto& layer : j["layers"]) 
        {
            if (!layer.is_object() || !layer.contains("name") || !layer["name"].is_string() ||
                !layer.contains("type") || !layer["type"].is_string()) {
                std::cerr << "Skipping invalid layer: Not an object or missing 'name'/'type' (string)" << std::endl;
                continue;
            }
            std::string layerName = layer["name"].get<std::string>();
            std::string layerType = layer["type"].get<std::string>();
            std::cout << "Processing layer: " << layerName << " (type: " << layerType << ")" << std::endl;

            if (layerType == "imagelayer") 
            {
                if (!layer.contains("image") || !layer["image"].is_string()) {
                    std::cerr << "Imagelayer " << layerName << " missing 'image' (string) field" << std::endl;
                    continue;
                }

                ImageLayer imgLayer;
                imgLayer.x = layer.value("x", 0);
                imgLayer.y = layer.value("y", 0);
                imgLayer.opacity = layer.value("opacity", 1.0f);
                imgLayer.parallaxX = layer.value("parallaxx", 1.0f);
                imgLayer.repeatX = layer.value("repeatx", false);
                imgLayer.offsetx = layer.value("offsetx", 0);
                imgLayer.offsety = layer.value("offsety", 0);

                std::string imgPath = "assets/" + layer["image"].get<std::string>();
                imgLayer.texture = IMG_LoadTexture(renderer, imgPath.c_str());
                if (!imgLayer.texture) {
                    std::cerr << "Failed to load imagelayer: " << layerName << " - " << IMG_GetError() << ", path: " << imgPath << std::endl;
                    continue;
                }

                if (layer.contains("imagewidth") && layer["imagewidth"].is_number_integer() &&
                    layer.contains("imageheight") && layer["imageheight"].is_number_integer()) {
                    imgLayer.imageWidth = layer["imagewidth"].get<int>();
                    imgLayer.imageHeight = layer["imageheight"].get<int>();
                } else {
                    SDL_QueryTexture(imgLayer.texture, nullptr, nullptr, &imgLayer.imageWidth, &imgLayer.imageHeight);
                }

                imageLayers.push_back(imgLayer);
                std::cout << "Loaded imagelayer: " << layerName << " (" << imgLayer.imageWidth << "x" << imgLayer.imageHeight << ")" << std::endl;
            }

            if ((layerName == "back" || layerName == "main") && layerType == "tilelayer") 
            {
                if (!layer.contains("data") || !layer["data"].is_array() ||
                    !layer.contains("width") || !layer["width"].is_number_integer() ||
                    !layer.contains("height") || !layer["height"].is_number_integer()) {
                    std::cerr << "Tilelayer " << layerName << " missing 'data' (array) or 'width'/'height' (integer)" << std::endl;
                    continue;
                }

                const auto& data = layer["data"];
                int layerWidth = layer["width"].get<int>();
                int layerHeight = layer["height"].get<int>();
                std::cout << "Tilelayer " << layerName << " size: " << layerWidth << "x" << layerHeight << " tiles" << std::endl;

                std::vector<std::vector<int>> tileLayer(layerHeight, std::vector<int>(layerWidth, 0));
                for (int y = 0; y < layerHeight; y++) {
                    for (int x = 0; x < layerWidth; x++) {
                        int index = y * layerWidth + x;
                        if (index >= (int)data.size() || !data[index].is_number_integer()) {
                            tileLayer[y][x] = 0;
                            continue;
                        }
                        tileLayer[y][x] = data[index].get<int>();
                    }
                }

                if (layerName == "back") {
                    backLayer = tileLayer;
                    std::cout << "Back layer loaded: " << backLayer.size() << " rows, " << backLayer[0].size() << " cols" << std::endl;
                } else if (layerName == "main") {
                    mainLayer = tileLayer;
                    std::cout << "Main layer loaded: " << mainLayer.size() << " rows, " << mainLayer[0].size() << " cols" << std::endl;
                }
            }
        }
    }

    // 6. 计算实际内容尺寸
    int layerWidth = mainLayer.empty() ? mapWidth : mainLayer[0].size();
    int layerHeight = mainLayer.empty() ? mapHeight : mainLayer.size();
    contentPixelWidth = layerWidth * tileWidth;
    contentPixelHeight = layerHeight * tileHeight;

    // 7. 标记危险瓦片（修改：现在已经在解析属性时自动标记）
    markTilesAsHazards();

    // 加载完成日志
    std::cout << "Map loading completed!" << std::endl;
    std::cout << "Content size: " << contentPixelWidth << "x" << contentPixelHeight << " pixels" << std::endl;
    std::cout << "Solid tiles count: " << solidTiles.size() << std::endl;
    std::cout << "Hazard tiles count: " << hazardTiles.size() << std::endl;
    std::cout << "Tilesets loaded: " << tilesetMap.size() << std::endl;
    std::cout << "Items tiles loaded: " << itemTextures.size() << std::endl;
}

void TiledMap::markTilesAsHazards() {
    std::cout << "=== 危险瓦片标记 ===" << std::endl;
    std::cout << "总危险瓦片数量: " << hazardTiles.size() << std::endl;
    for (int tileId : hazardTiles) {
        std::cout << "  - 危险瓦片 ID: " << tileId << std::endl;
    }
    
    if (hazardTiles.empty()) {
        std::cout << "警告: 没有找到任何危险瓦片!" << std::endl;
        std::cout << "请检查 Tiled 地图中是否有瓦片设置了 'Spike' 属性" << std::endl;
    } else {
        std::cout << "危险系统就绪。带有 'Spike' 属性的瓦片已自动标记为危险物。" << std::endl;
    }
}

// 其余 TiledMap.cpp 的代码保持不变...

TiledMap::~TiledMap() 
{
    for (auto& [name, tex] : tilesetMap) {
        SDL_DestroyTexture(tex);
    }
    for (auto& [id, tex] : itemTextures) {
        SDL_DestroyTexture(tex);
    }
    for (auto& layer : imageLayers) {
        SDL_DestroyTexture(layer.texture);
    }
}

// 危险物检测方法
bool TiledMap::isHazard(int worldX, int worldY) const {
    // 将世界坐标转换为原始坐标（考虑缩放）
    float originalX = (float)worldX / renderScale;
    float originalY = (float)worldY / renderScale;

    // 计算当前坐标对应的瓦片
    int tileX = (int)originalX / tileWidth;
    int tileY = (int)originalY / tileHeight;
    
    int layerWidth = mainLayer.empty() ? mapWidth : mainLayer[0].size();
    int layerHeight = mainLayer.empty() ? mapHeight : mainLayer.size();

    // 边界检查
    if (tileX < 0 || tileX >= layerWidth || tileY < 0 || tileY >= layerHeight) {
        return false;
    }

    // 检查瓦片是否为危险物
    int tileId = mainLayer[tileY][tileX];
    bool isHazard = hazardTiles.count(tileId) > 0;
    
    // 调试输出
    static int hazardDebugCount = 0;
    if (hazardDebugCount++ % 180 == 0 && isHazard) { // 每3秒输出一次危险物检测
        std::cout << "危险物检测 - 世界坐标: (" << worldX << ", " << worldY 
                  << ") -> 原始坐标: (" << originalX << ", " << originalY
                  << ") -> 瓦片坐标: (" << tileX << ", " << tileY 
                  << ") -> 瓦片ID: " << tileId << std::endl;
    }
    
    return isHazard;
}

void TiledMap::renderBackground(SDL_Renderer* renderer, const Camera& camera) const {
    for (const auto& layer : imageLayers) {
        renderImageLayer(renderer, camera, layer);
    }
}

void TiledMap::renderImageLayer(SDL_Renderer* renderer, const Camera& camera, const ImageLayer& layer) const {
    if (!layer.texture) return;
    const SDL_Rect& view = camera.getView();

    // 设置透明度
    if (layer.opacity < 1.0f) {
        SDL_SetTextureAlphaMod(layer.texture, (Uint8)(layer.opacity * 255));
    }

    // 计算视差偏移（x方向受 parallaxX 影响，y方向固定）
    int scaledContentWidth = contentPixelWidth * renderScale;
    int scaledContentHeight = contentPixelHeight * renderScale;
    int screenX = (int)(layer.x * renderScale + layer.offsetx - view.x * layer.parallaxX);
    int screenY = (int)(layer.y * renderScale + layer.offsety - view.y);
    int scaledImgW = (int)(layer.imageWidth * renderScale);
    int scaledImgH = (int)(layer.imageHeight * renderScale);

    // 重复渲染（水平方向）
    if (layer.repeatX && scaledImgW > 0) {
        int startOffset = screenX % scaledImgW;
        if (startOffset > 0) startOffset -= scaledImgW;  // 确保从左侧开始重复
        int totalRenderWidth = 0;
        while (totalRenderWidth < std::min(view.w + scaledImgW, scaledContentWidth)) {
            SDL_Rect dest = {
                screenX + startOffset + totalRenderWidth,
                screenY,
                scaledImgW,
                std::min(scaledImgH, scaledContentHeight)
            };
            SDL_RenderCopy(renderer, layer.texture, nullptr, &dest);
            totalRenderWidth += scaledImgW;
        }
    } else {
        // 单次渲染（不重复）
        SDL_Rect dest = {
            screenX,
            screenY,
            std::min(scaledImgW, scaledContentWidth),
            std::min(scaledImgH, scaledContentHeight)
        };
        SDL_RenderCopy(renderer, layer.texture, nullptr, &dest);
    }

    // 恢复透明度
    SDL_SetTextureAlphaMod(layer.texture, 255);
}

void TiledMap::renderTiles(SDL_Renderer* renderer, const Camera& camera) const {
    renderBackLayer(renderer, camera);
    renderMainLayer(renderer, camera);
}

void TiledMap::renderBackLayer(SDL_Renderer* renderer, const Camera& camera) const {
    if (backLayer.empty() || (tilesetMap.empty() && itemTextures.empty())) {
        std::cerr << "Back layer render skipped: empty layer or no textures" << std::endl;
        return;
    }

    const SDL_Rect& view = camera.getView();
    SDL_Rect srcRect = {0, 0, tileWidth, tileHeight};
    int layerWidth = backLayer[0].size();
    int layerHeight = backLayer.size();
    int scaledTileW = (int)(tileWidth * renderScale);
    int scaledTileH = (int)(tileHeight * renderScale);

    // 计算视野内的瓦片范围（避免渲染屏幕外瓦片，优化性能）
    int startX = std::max(0, view.x / scaledTileW);
    int startY = std::max(0, view.y / scaledTileH);
    int endX = std::min(layerWidth, (view.x + view.w) / scaledTileW + 1);  // +1 确保边缘瓦片渲染
    int endY = std::min(layerHeight, (view.y + view.h) / scaledTileH + 1);

    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            int tileId = backLayer[y][x];
            if (tileId == 0) continue;  // 跳过空白瓦片

            // 优先渲染 items 瓦片（单独纹理）
            auto itemTexIt = itemTextures.find(tileId);
            if (itemTexIt != itemTextures.end()) {
                auto sizeIt = itemSizes.find(tileId);
                if (sizeIt != itemSizes.end()) {
                    auto [imgW, imgH] = sizeIt->second;
                    // 计算 items 瓦片屏幕坐标（居中对齐到瓦片格子）
                    SDL_Rect dest = {
                        (int)(x * scaledTileW - view.x),
                        (int)(y * scaledTileH - view.y - (imgH * renderScale - scaledTileH)),
                        (int)(imgW * renderScale),
                        (int)(imgH * renderScale)
                    };
                    SDL_RenderCopy(renderer, itemTexIt->second, nullptr, &dest);
                }
                continue;
            }

            // 渲染普通瓦片（basic/Cave1）
            SDL_Texture* currentTileset = nullptr;
            int currentFirstGid = 0;
            // 查找当前瓦片所属的瓦片集（找最大的 firstGid ≤ tileId）
            for (const auto& [name, gid] : firstGidMap) {
                if (tileId >= gid && gid > currentFirstGid) {
                    auto texIt = tilesetMap.find(name);
                    if (texIt != tilesetMap.end() && texIt->second != nullptr) {
                        currentFirstGid = gid;
                        currentTileset = texIt->second;
                    }
                }
            }
            if (!currentTileset) {
                std::cerr << "Back layer: No tileset for tileId " << tileId << " at (" << x << "," << y << ")" << std::endl;
                continue;
            }

            // 计算瓦片在纹理中的坐标
            int localId = tileId - currentFirstGid;
            int tilesetW = 0, tilesetH = 0;
            SDL_QueryTexture(currentTileset, nullptr, nullptr, &tilesetW, &tilesetH);
            int tilesPerRow = tilesetW / tileWidth;
            int tilesPerCol = tilesetH / tileHeight;
            int maxLocalId = tilesPerRow * tilesPerCol - 1;

            // 边界检查（避免纹理越界）
            if (localId < 0 || localId > maxLocalId) {
                std::cerr << "Back layer: Invalid localId " << localId << " for tileId " << tileId << " (max: " << maxLocalId << ")" << std::endl;
                continue;
            }

            srcRect.x = (localId % tilesPerRow) * tileWidth;
            srcRect.y = (localId / tilesPerRow) * tileHeight;
            SDL_Rect dest = {
                (int)(x * scaledTileW - view.x),
                (int)(y * scaledTileH - view.y),
                scaledTileW,
                scaledTileH
            };
            SDL_RenderCopy(renderer, currentTileset, &srcRect, &dest);
        }
    }
}

void TiledMap::renderMainLayer(SDL_Renderer* renderer, const Camera& camera) const {
    if (mainLayer.empty() || (tilesetMap.empty() && itemTextures.empty())) {
        std::cerr << "Main layer render skipped: empty layer or no textures" << std::endl;
        return;
    }

    const SDL_Rect& view = camera.getView();
    SDL_Rect srcRect = {0, 0, tileWidth, tileHeight};
    int layerWidth = mainLayer[0].size();
    int layerHeight = mainLayer.size();
    int scaledTileW = (int)(tileWidth * renderScale);
    int scaledTileH = (int)(tileHeight * renderScale);

    int startX = std::max(0, view.x / scaledTileW);
    int startY = std::max(0, view.y / scaledTileH);
    int endX = std::min(layerWidth, (view.x + view.w) / scaledTileW + 1);
    int endY = std::min(layerHeight, (view.y + view.h) / scaledTileH + 1);

    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            int tileId = mainLayer[y][x];
            if (tileId == 0) continue;

            // 优先渲染 items 瓦片
            auto itemTexIt = itemTextures.find(tileId);
            if (itemTexIt != itemTextures.end()) {
                auto sizeIt = itemSizes.find(tileId);
                if (sizeIt != itemSizes.end()) {
                    auto [imgW, imgH] = sizeIt->second;
                    SDL_Rect dest = {
                        (int)(x * scaledTileW - view.x),
                        (int)(y * scaledTileH - view.y - (imgH * renderScale - scaledTileH)),
                        (int)(imgW * renderScale),
                        (int)(imgH * renderScale)
                    };
                    SDL_RenderCopy(renderer, itemTexIt->second, nullptr, &dest);
                }
                continue;
            }

            // 渲染普通瓦片
            SDL_Texture* currentTileset = nullptr;
            int currentFirstGid = 0;
            for (const auto& [name, gid] : firstGidMap) {
                if (tileId >= gid && gid > currentFirstGid) {
                    auto texIt = tilesetMap.find(name);
                    if (texIt != tilesetMap.end() && texIt->second != nullptr) {
                        currentFirstGid = gid;
                        currentTileset = texIt->second;
                    }
                }
            }
            if (!currentTileset) {
                std::cerr << "Main layer: No tileset for tileId " << tileId << " at (" << x << "," << y << ")" << std::endl;
                continue;
            }

            int localId = tileId - currentFirstGid;
            int tilesetW = 0;
            SDL_QueryTexture(currentTileset, nullptr, nullptr, &tilesetW, nullptr);
            int tilesPerRow = tilesetW / tileWidth;
            srcRect.x = (localId % tilesPerRow) * tileWidth;
            srcRect.y = (localId / tilesPerRow) * tileHeight;
            SDL_Rect dest = {
                (int)(x * scaledTileW - view.x),
                (int)(y * scaledTileH - view.y),
                scaledTileW,
                scaledTileH
            };
            SDL_RenderCopy(renderer, currentTileset, &srcRect, &dest);
        }
    }
}

// 碰撞检测（基于世界坐标，适配缩放）
bool TiledMap::isColliding(int worldX, int worldY) const {
    // 将缩放后的世界坐标转换为原始坐标
    float originalX = (float)worldX / renderScale;
    float originalY = (float)worldY / renderScale;

    // 计算当前坐标对应的瓦片
    int tileX = (int)originalX / tileWidth;
    int tileY = (int)originalY / tileHeight;
    int layerWidth = mainLayer.empty() ? mapWidth : mainLayer[0].size();
    int layerHeight = mainLayer.empty() ? mapHeight : mainLayer.size();

    // 边界检查（超出地图范围视为碰撞）
    if (tileX < 0 || tileX >= layerWidth || tileY < 0 || tileY >= layerHeight) {
        return true;
    }

    // 检查瓦片是否为固体
    int tileId = mainLayer[tileY][tileX];
    return solidTiles.count(tileId) > 0;
}
