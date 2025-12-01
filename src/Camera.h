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
        std::cout << "初始化相机 - 屏幕:" << screenW << "x" << screenH 
                  << ", 地图:" << mapPixelW << "x" << mapPixelH << std::endl;
    }

    // 跟随玩家（使玩家居中，限制在地图内）
    void follow(const glm::vec2& playerPos, float renderScale = 1.0f) {
        // 玩家位置已经应用了缩放，所以这里不需要再乘以 renderScale
        // 直接使用玩家位置的中心点
        glm::vec2 playerCenter = {
            playerPos.x + 8.0f,  // 玩家宽度16，中心在8
            playerPos.y + 12.0f  // 玩家高度24，中心在12
        };

        // 计算相机目标位置（玩家居中）
        float targetX = playerCenter.x - screenWidth / 2.0f;
        // 支持 Y 轴中心锁定：如果被锁定，使用 lockedCenterY 作为中心，否则使用玩家中心
        float centerY = yLocked ? lockedCenterY : playerCenter.y;
        float targetY = centerY - screenHeight / 2.0f;

        // 严格限制相机边界
        float minX = 0.0f;
        float minY = 0.0f;
        float maxX = std::max(0.0f, (float)(mapPixelWidth - screenWidth));
        float maxY = std::max(0.0f, (float)(mapPixelHeight - screenHeight));

        // 应用边界限制
        x = std::clamp(targetX, minX, maxX);
        y = std::clamp(targetY, minY, maxY);

        // 调试输出（减少频率）
        static int debugCount = 0;
        if (debugCount++ % 180 == 0) {
            std::cout << "=== 相机跟随 ===" << std::endl;
            std::cout << "玩家位置: (" << playerPos.x << ", " << playerPos.y << ")" << std::endl;
            std::cout << "玩家中心: (" << playerCenter.x << ", " << playerCenter.y << ")" << std::endl;
            std::cout << "相机目标: (" << targetX << ", " << targetY << ")" << std::endl;
            std::cout << "相机限制: X[" << minX << "~" << maxX << "], Y[" << minY << "~" << maxY << "]" << std::endl;
            std::cout << "相机最终: (" << x << ", " << y << ")" << std::endl;
            std::cout << "地图尺寸: " << mapPixelWidth << "x" << mapPixelHeight << std::endl;
            std::cout << "屏幕尺寸: " << screenWidth << "x" << screenHeight << std::endl;
        }
    }

    // 锁定相机 Y 轴的中心位置（传入世界坐标系下的中心 Y）
    void lockCenterY(float centerY) {
        yLocked = true;
        lockedCenterY = centerY;
    }

    // 解除 Y 轴锁定
    void unlockY() {
        yLocked = false;
    }

    // 查询是否锁定
    bool isYLocked() const { return yLocked; }

    // 一次性设置锁定中心并启用/禁用
    void setLockedCenterY(float centerY, bool enable) {
        if (enable) lockCenterY(centerY);
        else unlockY();
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
    bool yLocked = false;     // 是否锁定 Y 轴中心
    float lockedCenterY = 0.0f; // 被锁定时使用的中心 Y（世界像素坐标）
};