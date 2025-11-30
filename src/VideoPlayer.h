#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

class VideoPlayer {
public:
    VideoPlayer(SDL_Renderer* renderer);
    ~VideoPlayer();
    
    bool playDeathVideo();  // 播放死亡视频
    bool isPlaying() const;  // 只声明，不实现
    void stop();

private:
    SDL_Renderer* renderer;
    bool isVideoPlaying = false;
    Uint32 videoStartTime = 0;
    const Uint32 VIDEO_DURATION = 3000;  // 3秒视频
    
    // 可以添加纹理、音乐等资源
    SDL_Texture* deathTexture = nullptr;
    Mix_Chunk* deathSound = nullptr;
};