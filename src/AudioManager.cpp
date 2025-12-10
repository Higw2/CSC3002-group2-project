#include "AudioManager.h"

AudioManager::AudioManager() {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer初始化失败: " << Mix_GetError() << std::endl;
    } else {
        initialized = true;
        std::cout << "音频系统初始化成功" << std::endl;
    }
}

AudioManager::~AudioManager() {
    cleanup();
}

bool AudioManager::init() {
    return initialized;
}

void AudioManager::loadSound(const std::string& name, const std::string& filepath) {
    if (!initialized) return;
    
    auto it = sounds.find(name);
    if (it != sounds.end()) {
        Mix_FreeChunk(it->second);
        sounds.erase(it);
    }
    
    Mix_Chunk* chunk = Mix_LoadWAV(filepath.c_str());
    if (!chunk) {
        std::cerr << "无法加载音效 " << filepath << ": " << Mix_GetError() << std::endl;
        return;
    }
    
    sounds[name] = chunk;
    std::cout << "加载音效: " << name << " (" << filepath << ")" << std::endl;
}

void AudioManager::playSound(const std::string& name) {
    if (!initialized) return;
    
    auto it = sounds.find(name);
    if (it != sounds.end()) {
        Mix_PlayChannel(-1, it->second, 0);
    }
}

void AudioManager::stopSound(const std::string& name) {
    // 简化处理
}

void AudioManager::stopAll() {
    if (!initialized) return;
    Mix_HaltChannel(-1);  // 停止所有音效通道
}

void AudioManager::setVolume(const std::string& name, int volume) {
    if (!initialized) return;
    
    auto it = sounds.find(name);
    if (it != sounds.end()) {
        Mix_VolumeChunk(it->second, volume);
    }
}

void AudioManager::setMusicVolume(int volume) {
    if (!initialized) return;
    Mix_VolumeMusic(volume);  // 设置背景音乐音量
}

void AudioManager::setMasterVolume(int volume) {
    if (!initialized) return;
    Mix_Volume(-1, volume);
}

void AudioManager::playMusic(const std::string& filepath, int loops) {
    if (!initialized) return;
    
    stopMusic();
    
    currentMusic = Mix_LoadMUS(filepath.c_str());
    if (!currentMusic) {
        std::cerr << "无法加载音乐 " << filepath << ": " << Mix_GetError() << std::endl;
        return;
    }
    
    if (Mix_PlayMusic(currentMusic, loops) == -1) {
        std::cerr << "播放音乐失败: " << Mix_GetError() << std::endl;
    } else {
        std::cout << "开始播放音乐: " << filepath << std::endl;
    }
}

void AudioManager::stopMusic() {
    if (!initialized) return;
    
    if (currentMusic) {
        Mix_HaltMusic();
        Mix_FreeMusic(currentMusic);
        currentMusic = nullptr;
    }
}

void AudioManager::pauseMusic() {
    if (!initialized) return;
    Mix_PauseMusic();
}

void AudioManager::resumeMusic() {
    if (!initialized) return;
    Mix_ResumeMusic();
}

bool AudioManager::isMusicPlaying() const {
    if (!initialized) return false;
    return Mix_PlayingMusic() == 1;
}

void AudioManager::cleanup() {
    stopAll();
    stopMusic();
    
    for (auto& pair : sounds) {
        Mix_FreeChunk(pair.second);
    }
    sounds.clear();
    
    if (initialized) {
        Mix_CloseAudio();
        initialized = false;
    }
}