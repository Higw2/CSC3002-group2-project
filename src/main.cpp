#include <SDL2/SDL.h>
#include <vector>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <string>

// 游戏常量
const int WIDTH = 640;
const int HEIGHT = 480;
const int SIZE = 32;
const float GRAVITY = 0.8f;
const int JUMP = -15;
const int SPEED = 5;
const int COIN_SIZE = 16;
const int MONSTER_SPEED = 2;

// 碰撞检测
bool checkCollision(SDL_Rect a, SDL_Rect b) {
    return a.x < b.x + b.w &&
           a.x + a.w > b.x &&
           a.y < b.y + b.h &&
           a.y + a.h > b.y;
}

// 绘制简单分数
void drawScore(SDL_Renderer* renderer, int score, int x, int y) {
    // 用多个小方块表示分数
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    for (int i = 0; i < score / 10; i++) {
        SDL_Rect rect = {x + i * 10, y, 8, 8};
        SDL_RenderFillRect(renderer, &rect);
    }
}

int main(int argc, char* argv[]) {
    // 初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return 1;
    }

    // 创建窗口和渲染器
    SDL_Window* window = SDL_CreateWindow(
        "demo",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT, SDL_WINDOW_SHOWN
    );
    if (!window) {
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // 玩家
    SDL_Rect player = {50, 50, SIZE, SIZE};
    float ySpeed = 0;
    bool onGround = false;
    bool alive = true;

    // 平台
    std::vector<SDL_Rect> platforms = {
        {0, HEIGHT - SIZE, WIDTH, SIZE},  // 地面
        {150, 350, SIZE*2, SIZE},
        {350, 250, SIZE*2, SIZE},
        {500, 150, SIZE, SIZE}
    };

    // 金币
    struct Coin {
        SDL_Rect rect;
        bool collected;
    };
    std::vector<Coin> coins;
    coins.push_back({{180, 320, COIN_SIZE, COIN_SIZE}, false});
    coins.push_back({{400, 220, COIN_SIZE, COIN_SIZE}, false});
    coins.push_back({{510, 120, COIN_SIZE, COIN_SIZE}, false});
    coins.push_back({{200, 320, COIN_SIZE, COIN_SIZE}, false});

    // 怪物
    struct Monster {
        SDL_Rect rect;
        int dir; // 1=右, -1=左
    };
    std::vector<Monster> monsters;
    monsters.push_back({{300, 350 - SIZE, SIZE, SIZE}, 1});
    monsters.push_back({{450, 250 - SIZE, SIZE, SIZE}, -1});

    int score = 0;
    bool running = true;
    SDL_Event event;

    //-----------------------------加入测试音乐，检测库能不能用——黄子恩
    //初始化音频
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) 
    {  // 增加SDL_INIT_AUDIO
    std::cout << "SDL_audio初始化失败: " << SDL_GetError() << std::endl;
    return 1;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) 
    {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        return 1;
    }
    Mix_Music *bgm = Mix_LoadMUS("assets/audio/test.wav"); //读取音乐文件
    Mix_PlayMusic(bgm, -1); //播放音乐，-1表示循环播放

    //------------------------------加入测试音乐结束——黄子恩

    //-----------------------------加入背景，检测库能不能用png文件———黄子恩
    //初始化SDL2_image
    if (IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) != IMG_INIT_PNG | IMG_INIT_JPG)
    {
        std::cout << "SDL_image初始化失败: " << SDL_GetError() << std::endl;
    }
    //加载图片,三重图片叠加形成完整的背景
    SDL_Texture *texture1 = IMG_LoadTexture(renderer, "assets/sprites/test.1.png");
    SDL_Texture *texture2 = IMG_LoadTexture(renderer, "assets/sprites/test.2.png");
    SDL_Texture *texture3 = IMG_LoadTexture(renderer, "assets/sprites/test.3.png");
    //画图片部分在主循环中

    while (running) {
        // 事件处理
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            // 重置游戏
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_r && !alive) {
                    player = {50, 50, SIZE, SIZE};
                    alive = true;
                    score = 0;
                    for (auto& c : coins) c.collected = false;
                }
            }
        }

        if (alive) {
            // 玩家控制
            const Uint8* keys = SDL_GetKeyboardState(NULL);
            if (keys[SDL_SCANCODE_LEFT] && player.x > 0) {
                player.x -= SPEED;
            }
            if (keys[SDL_SCANCODE_RIGHT] && player.x < WIDTH - SIZE) {
                player.x += SPEED;
            }
            if (keys[SDL_SCANCODE_UP] && onGround) {
                ySpeed = JUMP;
                onGround = false;
            }

            // 重力应用
            ySpeed += GRAVITY;
            player.y += (int)ySpeed;

            // 平台碰撞
            onGround = false;
            for (size_t i = 0; i < platforms.size(); i++) {
                SDL_Rect p = platforms[i];
                if (checkCollision(player, p)) {
                    if (ySpeed > 0) { // 下落碰撞
                        player.y = p.y - player.h;
                        ySpeed = 0;
                        onGround = true;
                    } else if (ySpeed < 0) { // 上跳碰撞
                        player.y = p.y + p.h;
                        ySpeed = 0;
                    }
                }
            }

            // 金币收集
            for (size_t i = 0; i < coins.size(); i++) {
                if (!coins[i].collected && checkCollision(player, coins[i].rect)) {
                    coins[i].collected = true;
                    score += 10;
                }
            }

            // 怪物移动
            for (size_t i = 0; i < monsters.size(); i++) {
                // 移动
                monsters[i].rect.x += monsters[i].dir * MONSTER_SPEED;

                // 边界检测（反转方向）
                if (monsters[i].rect.x <= 0 || monsters[i].rect.x + SIZE >= WIDTH) {
                    monsters[i].dir *= -1;
                }

                // 怪物与玩家碰撞
                if (checkCollision(player, monsters[i].rect)) {
                    alive = false;
                }
            }
        }

        /*---------------更改背景图片--黄子恩
        // 渲染
        SDL_SetRenderDrawColor(renderer, 135, 206, 235, 255); // 天空蓝
        SDL_RenderClear(renderer);                                         */
        SDL_RenderCopy(renderer, texture1,NULL,NULL);
        SDL_RenderCopy(renderer, texture2,NULL,NULL);
        SDL_RenderCopy(renderer, texture3,NULL,NULL);


        // 绘制平台（棕色）
        SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255);
        for (size_t i = 0; i < platforms.size(); i++) {
            SDL_RenderFillRect(renderer, &platforms[i]);
        }

        // 绘制金币（黄色）
        SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
        for (size_t i = 0; i < coins.size(); i++) {
            if (!coins[i].collected) {
                SDL_RenderFillRect(renderer, &coins[i].rect);
            }
        }

        // 绘制怪物（绿色）
        SDL_SetRenderDrawColor(renderer, 0, 128, 0, 255);
        for (size_t i = 0; i < monsters.size(); i++) {
            SDL_RenderFillRect(renderer, &monsters[i].rect);
        }

        // 绘制玩家（红色，死亡为灰色）
        if (alive) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        }
        SDL_RenderFillRect(renderer, &player);

        // 绘制分数
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // 白色背景
        SDL_Rect scoreBgRect = {10, 10, 100, 20}; // 先创建SDL_Rect变量
        SDL_RenderFillRect(renderer, &scoreBgRect);
        drawScore(renderer, score, 15, 15);

        // 死亡提示
        if (!alive) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // 红色背景
            SDL_Rect deathBgRect = {WIDTH / 2 - 80, HEIGHT / 2 - 20, 160, 40}; // 先创建SDL_Rect变量
            SDL_RenderFillRect(renderer, &deathBgRect);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // 白色文字块
            SDL_Rect deathTextRect = {WIDTH / 2 - 60, HEIGHT / 2 - 10, 40, 20}; // 先创建SDL_Rect变量
            SDL_RenderFillRect(renderer, &deathTextRect);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);

        
    }

    // 清理资源
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    //清理音乐资源
    Mix_FreeMusic(bgm);
    Mix_CloseAudio();
    Mix_Quit();
    //清理图片背景资源
    SDL_DestroyTexture(texture1);
    SDL_DestroyTexture(texture2);
    SDL_DestroyTexture(texture3);
    IMG_Quit;

    SDL_Quit();

    return 0;
}