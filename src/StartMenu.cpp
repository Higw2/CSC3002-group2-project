#include "StartMenu.h"

#include <iostream>

StartMenu::StartMenu(SDL_Renderer* rend) : renderer(rend) {}

StartMenu::~StartMenu() {
    cleanup();
}

bool StartMenu::init() {
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf 初始化失败: " << TTF_GetError() << std::endl;
        return false;
    }

    if (!loadBackground()) {
        std::cerr << "背景图加载失败，使用纯色背景" << std::endl;
    }

    loadFont();
    if (!font) {
        std::cerr << "字体加载失败" << std::endl;
        return false;
    }

    int startY = 125;
    int itemSpacing = 60;

    createMenuItem("开始游戏", startY);
    createMenuItem("继续游戏", startY + itemSpacing);
    createMenuItem("退出游戏", startY + itemSpacing * 2);

    if (!menuItems.empty()) {
        menuItems[0].isSelected = true;
    }

    return true;
}

bool StartMenu::loadBackground() {
    backgroundTexture = IMG_LoadTexture(renderer, "assets/menu/background.png");
    if (!backgroundTexture) {
        std::cerr << "无法加载背景图: " << IMG_GetError() << std::endl;
        return false;
    }
    return true;
}

void StartMenu::loadFont() {
    font = TTF_OpenFont("assets/fonts/FLyouzichati-Regular-2.ttf", 32);
    if (!font) {
        font = TTF_OpenFont("arial.ttf", 32);
        if (!font) {
            std::cerr << "无法加载字体" << std::endl;
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

        int textWidth, textHeight;
        SDL_QueryTexture(item.texture, nullptr, nullptr, &textWidth, &textHeight);
        item.rect = {400 - textWidth / 2, yPos, textWidth, textHeight};
    } else {
        item.rect = {300, yPos, 200, 40};
    }

    menuItems.push_back(item);
}

void StartMenu::handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            quitMenu = true;
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_w:
                case SDLK_UP:
                    menuItems[currentSelection].isSelected = false;
                    currentSelection = (currentSelection - 1 + menuItems.size()) % menuItems.size();
                    menuItems[currentSelection].isSelected = true;
                    break;

                case SDLK_s:
                case SDLK_DOWN:
                    menuItems[currentSelection].isSelected = false;
                    currentSelection = (currentSelection + 1) % menuItems.size();
                    menuItems[currentSelection].isSelected = true;
                    break;

                case SDLK_RETURN:
                case SDLK_SPACE:
                    quitMenu = true;
                    break;

                case SDLK_ESCAPE:
                    currentSelection = menuItems.size() - 1;
                    quitMenu = true;
                    break;
            }
        }
    }
}

void StartMenu::render() {
    SDL_RenderClear(renderer);

    if (backgroundTexture) {
        SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);
    } else {
        SDL_SetRenderDrawColor(
            renderer, COLOR_BACKGROUND.r, COLOR_BACKGROUND.g, COLOR_BACKGROUND.b, 255);
        SDL_RenderClear(renderer);
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
    SDL_Rect overlay = {0, 0, 800, 350};
    SDL_RenderFillRect(renderer, &overlay);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

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

    for (auto& item : menuItems) {
        if (item.texture) {
            SDL_DestroyTexture(item.texture);

            SDL_Color color = item.isSelected ? COLOR_SELECTED : COLOR_NORMAL;
            SDL_Surface* surface = TTF_RenderUTF8_Blended(font, item.text.c_str(), color);
            item.texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);

            SDL_RenderCopy(renderer, item.texture, nullptr, &item.rect);
        } else {
            if (item.isSelected) {
                SDL_SetRenderDrawColor(
                    renderer, COLOR_SELECTED.r, COLOR_SELECTED.g, COLOR_SELECTED.b, 255);
            } else {
                SDL_SetRenderDrawColor(
                    renderer, COLOR_NORMAL.r, COLOR_NORMAL.g, COLOR_NORMAL.b, 255);
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
        SDL_Delay(16);
    }

    std::cout << "菜单选择: " << currentSelection << std::endl;

    switch (currentSelection) {
        case 0:
            return 0;
        case 1:
            return 1;
        case 2:
            return 2;
        default:
            return 0;
    }
}

void StartMenu::cleanup() {
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
    for (size_t i = 0; i < menuItems.size(); ++i) {
        menuItems[i].isSelected = (i == 0);
    }
}
