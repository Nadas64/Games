#include "Game.hpp"

int main()
{
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Tetris - raylib");
    SetTargetFPS(60);

    Game game;

    while (!WindowShouldClose())
    {
        game.Update();
        BeginDrawing();
        game.Draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
