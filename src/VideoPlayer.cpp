#include "VideoPlayer.h"
#include <iostream>

VideoPlayer::VideoPlayer(SDL_Renderer* rend) : renderer(rend) {
    // 可以在这里加载死亡特效资源
    std::cout << "VideoPlayer initialized" << std::endl;
}

VideoPlayer::~VideoPlayer() {
    stop();
    if (deathTexture) {
        SDL_DestroyTexture(deathTexture);
    }
    if (deathSound) {
        Mix_FreeChunk(deathSound);
    }
}

bool VideoPlayer::playDeathVideo() {
    if (isVideoPlaying) return false;
    
    isVideoPlaying = true;
    videoStartTime = SDL_GetTicks();
    
    std::cout << "开始播放死亡动画..." << std::endl;
    // 这里估计要引入更多的库才能播放视频
    // 暂时用简单的计时器模拟
    
    return true;
}

bool VideoPlayer::isPlaying() const {
    if (!isVideoPlaying) return false;
    
    // 模拟3秒的视频播放
    Uint32 currentTime = SDL_GetTicks();
    return (currentTime - videoStartTime) < VIDEO_DURATION;
}

void VideoPlayer::stop() {
    isVideoPlaying = false;
    videoStartTime = 0;
}