#include "Game.h"
#include <SDL2/SDL.h>
#include <windows.h>

int main(int argc, char* argv[]) 
{
    SetConsoleOutputCP(CP_UTF8);// 设置控制台为UTF-8编码，支持中文输出
    Game game;
    game.run();
    return 0;
}