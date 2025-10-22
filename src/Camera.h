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
        std::cout << "相机初始化: 屏幕=" << screenW << "x" << screenH 
                  << ", 地图=" << mapPixelW << "x" << mapPixelH << std::endl;
    }

    // 跟随玩家（使玩家居中，限制在地图内）
    void follow(const glm::vec2& playerPos, float renderScale = 1.0f) {
        // 玩家中心坐标（考虑缩放后的玩家尺寸）
        glm::vec2 playerCenter = {
            playerPos.x * renderScale + (16 * renderScale) / 2.0f,
            playerPos.y * renderScale + (24 * renderScale) / 2.0f
        };

        std::cout << "玩家中心坐标(缩放后): (" << playerCenter.x << ", " << playerCenter.y << ")" << std::endl;

        // 计算相机目标位置（玩家居中）
        float targetX = playerCenter.x - screenWidth / 2.0f;
        float targetY = playerCenter.y - screenHeight / 2.0f;

        std::cout << "相机目标位置: (" << targetX << ", " << targetY << ")" << std::endl;

        // 严格限制相机边界
        float minX = 0.0f;
        float minY = 0.0f;
        float maxX = std::max(0.0f, (float)(mapPixelWidth - screenWidth));
        float maxY = std::max(0.0f, (float)(mapPixelHeight - screenHeight));

        std::cout << "相机边界范围: X[" << minX << "~" << maxX << "], Y[" << minY << "~" << maxY << "]" << std::endl;

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

        std::cout << "相机最终位置: (" << x << ", " << y << ")" << std::endl;
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