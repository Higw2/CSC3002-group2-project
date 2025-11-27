#include "StartMenu.h"
#include <iostream>

StartMenu::StartMenu(SDL_Renderer* rend) : renderer(rend) {}

StartMenu::~StartMenu() {
    cleanup();
}

bool StartMenu::init() {
    // 初始化SDL_ttf
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf初始化失败: " << TTF_GetError() << std::endl;
        return false;
    }

    // 加载字体
    loadFont();
    if (!font) {
        std::cerr << "字体加载失败" << std::endl;
        return false;
    }

    // 创建菜单项
    int startY = 200;
    int itemSpacing = 60;
    
    createMenuItem("开始游戏", startY);
    createMenuItem("继续游戏", startY + itemSpacing);
    createMenuItem("设置", startY + itemSpacing * 2);
    createMenuItem("退出游戏", startY + itemSpacing * 3);

    // 设置初始选择
    if (!menuItems.empty()) {
        menuItems[0].isSelected = true;
    }

    return true;
}

void StartMenu::loadFont() {
    // 尝试加载字体文件，你可以替换为你的游戏字体
    font = TTF_OpenFont("assets/fonts/arial.ttf", 32);
    if (!font) {
        // 如果指定字体不存在，尝试系统默认字体
        font = TTF_OpenFont("arial.ttf", 32);
        if (!font) {
            std::cerr << "无法加载字体，使用备用方案" << std::endl;
        }
    }
}

void StartMenu::createMenuItem(const std::string& text, int yPos) {
    MenuItem item;
    item.text = text;
    
    if (font) {
        SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), COLOR_NORMAL);
        item.texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        
        // 获取文本尺寸并设置位置（居中）
        int textWidth, textHeight;
        SDL_QueryTexture(item.texture, nullptr, nullptr, &textWidth, &textHeight);
        item.rect = {400 - textWidth / 2, yPos, textWidth, textHeight};
    } else {
        // 字体加载失败的备用方案 - 使用矩形表示菜单项
        item.rect = {300, yPos, 200, 40};
    }
    
    menuItems.push_back(item);
}

void StartMenu::handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            quitMenu = true;
        }
        else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_w:
                case SDLK_UP:
                    // 向上选择
                    menuItems[currentSelection].isSelected = false;
                    currentSelection = (currentSelection - 1 + menuItems.size()) % menuItems.size();
                    menuItems[currentSelection].isSelected = true;
                    break;
                    
                case SDLK_s:
                case SDLK_DOWN:
                    // 向下选择
                    menuItems[currentSelection].isSelected = false;
                    currentSelection = (currentSelection + 1) % menuItems.size();
                    menuItems[currentSelection].isSelected = true;
                    break;
                    
                case SDLK_RETURN:
                case SDLK_SPACE:
                    // 确认选择
                    quitMenu = true;
                    break;
                    
                case SDLK_ESCAPE:
                    // 退出菜单
                    currentSelection = menuItems.size() - 1; // 选择退出项
                    quitMenu = true;
                    break;
            }
        }
    }
}

void StartMenu::render() {
    // 设置背景色（与游戏背景一致）
    SDL_SetRenderDrawColor(renderer, COLOR_BACKGROUND.r, COLOR_BACKGROUND.g, COLOR_BACKGROUND.b, 255);
    SDL_RenderClear(renderer);
    
    // 渲染菜单标题
    if (font) {
        SDL_Surface* titleSurface = TTF_RenderUTF8_Blended(font, "EchoRidge", COLOR_SELECTED);
        SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(renderer, titleSurface);
        int titleWidth, titleHeight;
        SDL_QueryTexture(titleTexture, nullptr, nullptr, &titleWidth, &titleHeight);
        
        SDL_Rect titleRect = {400 - titleWidth / 2, 100, titleWidth, titleHeight};
        SDL_RenderCopy(renderer, titleTexture, nullptr, &titleRect);
        
        SDL_DestroyTexture(titleTexture);
        SDL_FreeSurface(titleSurface);
    }
    
    // 渲染菜单项
    for (auto& item : menuItems) {
        if (item.texture) {
            // 如果有纹理，使用纹理渲染
            if (item.isSelected) {
                // 重新创建选中状态的纹理
                SDL_DestroyTexture(item.texture);
                SDL_Surface* surface = TTF_RenderUTF8_Blended(font, item.text.c_str(), COLOR_SELECTED);
                item.texture = SDL_CreateTextureFromSurface(renderer, surface);
                SDL_FreeSurface(surface);
            }
            SDL_RenderCopy(renderer, item.texture, nullptr, &item.rect);
        } else {
            // 备用方案：绘制彩色矩形
            if (item.isSelected) {
                SDL_SetRenderDrawColor(renderer, COLOR_SELECTED.r, COLOR_SELECTED.g, COLOR_SELECTED.b, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, COLOR_NORMAL.r, COLOR_NORMAL.g, COLOR_NORMAL.b, 255);
            }
            SDL_RenderFillRect(renderer, &item.rect);
        }
    }
    
    SDL_RenderPresent(renderer);
}

int StartMenu::run() {
    std::cout << "开始菜单启动" << std::endl;
    
    while (!quitMenu) {
        handleEvents();
        render();
        SDL_Delay(16); // 约60FPS
    }
    
    std::cout << "菜单选择: " << currentSelection << std::endl;
    return currentSelection;
}

void StartMenu::cleanup() {
    for (auto& item : menuItems) {
        if (item.texture) {
            SDL_DestroyTexture(item.texture);
        }
    }
    menuItems.clear();
    
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
    
    TTF_Quit();
}