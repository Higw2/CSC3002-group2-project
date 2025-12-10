#pragma once
#include <SDL2/SDL_mixer.h>
#include <string>
#include <unordered_map>
#include <iostream>

class AudioManager {
public:
    AudioManager();
    ~AudioManager();
    
    bool init();
    void loadSound(const std::string& name, const std::string& filepath);
    void playSound(const std::string& name);
    void stopSound(const std::string& name);
    void setVolume(const std::string& name, int volume);
    void setMasterVolume(int volume);
    void setMusicVolume(int volume);
    void playMusic(const std::string& filepath, int loops = -1);
    void stopMusic();
    void stopAll();
    void pauseMusic();
    void resumeMusic();
    bool isMusicPlaying() const;
    
    void cleanup();

private:
    std::unordered_map<std::string, Mix_Chunk*> sounds;
    Mix_Music* currentMusic = nullptr;
    bool initialized = false;
};