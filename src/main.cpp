#include "Game.h"
#include <SDL2/SDL.h>
#include <windows.h>

int main(int argc, char* argv[]) 
{
    SetConsoleOutputCP(CP_UTF8);
    Game game;
    game.run();
    return 0;
}