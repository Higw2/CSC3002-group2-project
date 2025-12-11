#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <string>
#include <vector>

class StartMenu {
public:
    StartMenu(SDL_Renderer* renderer);
    ~StartMenu();

    bool init();
    int run();  // 返回用户选择
    void cleanup();
    void reset();  // 重置选项状态

private:
    struct MenuItem {
        std::string text;
        SDL_Texture* texture = nullptr;
        SDL_Rect rect;
        bool isSelected = false;
    };

    SDL_Renderer* renderer;
    TTF_Font* font = nullptr;
    SDL_Texture* backgroundTexture = nullptr;
    std::vector<MenuItem> menuItems;
    int currentSelection = 0;
    bool quitMenu = false;

    // 颜色常量
    const SDL_Color COLOR_NORMAL = {255, 255, 255, 255};
    const SDL_Color COLOR_SELECTED = {0, 200, 255, 255};
    const SDL_Color COLOR_BACKGROUND = {30, 30, 30, 255};

    void handleEvents();
    void render();
    void createMenuItem(const std::string& text, int yPos);
    void loadFont();
    bool loadBackground();
};
