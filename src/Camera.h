#pragma once
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <algorithm>
#include <iostream>

class Camera {
public:
    Camera(int screenW, int screenH, int mapPixelW, int mapPixelH)
        : screenWidth(screenW), screenHeight(screenH),
          mapPixelWidth(mapPixelW), mapPixelHeight(mapPixelH) {
        std::cout << "init the camera =" << screenW << "x" << screenH 
                  << ", maps =" << mapPixelW << "x" << mapPixelH << std::endl;
    }

    // 跟随玩家（使玩家居中，限制在地图内）
    void follow(const glm::vec2& playerPos, float renderScale = 1.0f) {
        // 玩家中心坐标（考虑缩放后的玩家尺寸）现在是缩放了2倍的
        glm::vec2 playerCenter = {
            playerPos.x * renderScale + (16 * renderScale) / 2.0f,
            playerPos.y * renderScale + (24 * renderScale) / 2.0f
        };

        std::cout << "thr position of player(after dealing): (" << playerCenter.x << ", " << playerCenter.y << ")" << std::endl;

        // 计算相机目标位置（玩家居中）
        float targetX = playerCenter.x - screenWidth / 2.0f;
        float targetY = playerCenter.y - screenHeight / 2.0f;

        std::cout << "the target position of the camera: (" << targetX << ", " << targetY << ")" << std::endl;

        // 严格限制相机边界
        float minX = 0.0f;
        float minY = 0.0f;
        float maxX = std::max(0.0f, (float)(mapPixelWidth - screenWidth));
        float maxY = std::max(0.0f, (float)(mapPixelHeight - screenHeight));

        std::cout << "the edge of the camer: X[" << minX << "~" << maxX << "], Y[" << minY << "~" << maxY << "]" << std::endl;

        // 水平方向
        if (mapPixelWidth <= screenWidth) {
            x = (mapPixelWidth - screenWidth) / 2.0f;
        } else {
            x = std::clamp(targetX, minX, maxX);
        }

        // 垂直方向
        if (mapPixelHeight <= screenHeight) {
            y = (mapPixelHeight - screenHeight) / 2.0f;
        } else {
            y = std::clamp(targetY, minY, maxY);
        }

        std::cout << "the fianl position of the camera: (" << x << ", " << y << ")" << std::endl;
    }

    SDL_Rect getView() const {
        return { (int)x, (int)y, screenWidth, screenHeight };
    }

    float x = 0;         // 相机左上角X（地图像素坐标）
    float y = 0;         // 相机左上角Y（地图像素坐标）
    const int screenWidth;    // 屏幕宽度
    const int screenHeight;   // 屏幕高度
    const int mapPixelWidth;  // 地图像素宽度
    const int mapPixelHeight; // 地图像素高度
};