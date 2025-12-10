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

    // 加载背景图片
    if (!loadBackground()) {
        std::cerr << "背景图片加载失败，将使用纯色背景" << std::endl;
        // 不返回false，允许继续使用纯色背景
    }

    // 加载字体
    loadFont();
    if (!font) {
        std::cerr << "字体加载失败" << std::endl;
        return false;
    }

    // 创建菜单项 - 移除"设置"选项
    int startY = 125;
    int itemSpacing = 60;
    
    createMenuItem("开始游戏", startY);
    createMenuItem("继续游戏", startY + itemSpacing);
    createMenuItem("退出游戏", startY + itemSpacing * 2);  // 调整间距

    // 设置初始选择
    if (!menuItems.empty()) {
        menuItems[0].isSelected = true;
    }

    return true;
}

bool StartMenu::loadBackground() {
    // 尝试加载背景图片，你可以替换为你想要的背景图片路径
    backgroundTexture = IMG_LoadTexture(renderer, "assets/menu/background.png");
    if (!backgroundTexture) {
            if (!backgroundTexture) {
                std::cerr << "无法加载背景图片: " << IMG_GetError() << std::endl;
                return false;
            }
        }
    std::cout << "背景图片加载成功" << std::endl;
    return true;
}

void StartMenu::loadFont() {
    // 尝试加载字体文件，你可以替换为你的游戏字体
    font = TTF_OpenFont("assets/fonts/FLyouzichati-Regular-2.ttf", 32);
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
    // 清除渲染器
    SDL_RenderClear(renderer);
    
    // 渲染背景
    if (backgroundTexture) {
        // 如果有背景图片，渲染全屏背景
        SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);
    } else {
        // 如果没有背景图片，使用纯色背景
        SDL_SetRenderDrawColor(renderer, COLOR_BACKGROUND.r, COLOR_BACKGROUND.g, COLOR_BACKGROUND.b, 255);
        SDL_RenderClear(renderer);
    }
    
    // 渲染半透明黑色覆盖层，提高文字可读性（可选）
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128); // 半透明黑色
    SDL_Rect overlay = {0, 0, 800, 350}; // 覆盖整个屏幕
    SDL_RenderFillRect(renderer, &overlay);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    
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
            // 先销毁旧纹理
            SDL_DestroyTexture(item.texture);
            
            // 根据选择状态创建新纹理
            SDL_Color color = item.isSelected ? COLOR_SELECTED : COLOR_NORMAL;
            SDL_Surface* surface = TTF_RenderUTF8_Blended(font, item.text.c_str(), color);
            item.texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            
            // 渲染纹理
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
    
    // 由于移除了设置选项，需要调整返回值映射
    switch(currentSelection) {
        case 0: return 0; // 开始游戏
        case 1: return 1; // 继续游戏  
        case 2: return 2; // 退出游戏 (原来是设置)
        default: return 0;
    }
}

void StartMenu::cleanup() {
    // 清理背景纹理
    if (backgroundTexture) {
        SDL_DestroyTexture(backgroundTexture);
        backgroundTexture = nullptr;
    }
    
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

void StartMenu::reset() {
    quitMenu = false;
    currentSelection = 0;
    // 重置选择状态
    for (size_t i = 0; i < menuItems.size(); ++i) {
        menuItems[i].isSelected = (i == 0);
    }
}