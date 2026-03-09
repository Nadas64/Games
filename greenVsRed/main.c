#include "raylib.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

//galima kaitalioti
#define WINDOW_SIZE 400
#define PADDING 20
#define SIZE 1

#define ROWS ((WINDOW_SIZE - 2 * PADDING) / SIZE)
#define COLS ((WINDOW_SIZE - 2 * PADDING) / SIZE)

long long scoreGreen = 1;
long long scoreRed = 1;

int random()
{
    int accel = 0;
    if ((rand() % 5) == 0) accel++;

    if (rand() % 11)
        return ((rand() % 41) - accel < 2) ? 2 : (rand() % 41) - accel;

    int v = (rand() % 301) + 100 - accel;
    return (v < 2) ? 2 : v;
}

int main()
{
    srand((unsigned)time(0));

    double first = 0;
    double last = 0;

    int random1 = 50, random2 = 50;
    int greenActive = 1, redActive = 1;

    char board[ROWS][COLS];
    memset(board, 'E', sizeof(board));

    InitWindow(WINDOW_SIZE, WINDOW_SIZE, "corruption");
    SetTargetFPS(144);

    board[0][0] = 'G';
    board[ROWS - 1][COLS - 1] = 'R';

    while (!WindowShouldClose())
    {
        last = GetTime();
        if ((last - first) > 5.0f)
        {
            if (greenActive) random1 = random();
            if (redActive)   random2 = random();
            first = GetTime();
        }

        BeginDrawing();
        ClearBackground(BLACK);

        int greenCanMove = 0, redCanMove = 0;
        for (int i = 0; i < ROWS; ++i)
        {
            for (int j = 0; j < COLS; ++j)
            {
                if (!greenCanMove && board[i][j] == 'G')
                {
                    if ((i + 1 < ROWS && board[i + 1][j] == 'E') ||
                        (j + 1 < COLS && board[i][j + 1] == 'E'))
                        greenCanMove = 1;
                }
                if (!redCanMove && board[i][j] == 'R')
                {
                    if ((i - 1 >= 0 && board[i - 1][j] == 'E') ||
                        (j - 1 >= 0 && board[i][j - 1] == 'E'))
                        redCanMove = 1;
                }
            }
        }

        if (!redCanMove && scoreGreen > scoreRed) { redActive = 0; random1 = 2; }
        if (!greenCanMove && scoreRed > scoreGreen) { greenActive = 0; random2 = 2; }

        for (int i = 0; i < ROWS; ++i)
        {
            for (int j = 0; j < COLS; ++j)
            {
                if (board[i][j] == 'G')
                {
                    if (greenActive && (rand() % random1) == 0)
                    {
                        if (i + 1 < ROWS && board[i + 1][j] == 'E')
                            board[i + 1][j] = 'G', ++scoreGreen;
                        if (j + 1 < COLS && board[i][j + 1] == 'E')
                            board[i][j + 1] = 'G', ++scoreGreen;
                    }
                    DrawRectangle(PADDING + j * SIZE, PADDING + i * SIZE, SIZE, SIZE, GREEN);
                }
                else if (board[i][j] == 'R')
                {
                    if (redActive && (rand() % random2) == 0)
                    {
                        if (i - 1 >= 0 && board[i - 1][j] == 'E')
                            board[i - 1][j] = 'R', ++scoreRed;
                        if (j - 1 >= 0 && board[i][j - 1] == 'E')
                            board[i][j - 1] = 'R', ++scoreRed;
                    }
                    DrawRectangle(PADDING + j * SIZE, PADDING + i * SIZE, SIZE, SIZE, RED);
                }
            }
        }

        char s1[32], s2[32];
        snprintf(s1, sizeof(s1), "%lld", scoreGreen);
        snprintf(s2, sizeof(s2), "%lld", scoreRed);

        DrawText("Green Count:", 90, 30, 20, DARKBLUE);
        DrawText("Red Count:",   90, 50, 20, DARKBLUE);
        DrawText(s1, 230, 30, 20, BLUE);
        DrawText(s2, 230, 50, 20, BLUE);

        if ((scoreGreen + scoreRed) >= (long long)ROWS * (long long)COLS)
        {
            const char *msg = (scoreGreen > scoreRed) ? "GREEN WINS!!!" : "RED WINS!!!";
            const char offset = (scoreGreen > scoreRed) ? 30 : 60;
            Color winCol = (scoreGreen > scoreRed) ? GREEN : RED;

            while (!WindowShouldClose())
            {
                int blink = ((int)(GetTime() * 2.0)) & 1;
                BeginDrawing();
                ClearBackground(BLACK);
                DrawText(msg, offset, 140, 50, blink ? winCol : WHITE);
                EndDrawing();
            }

            EndDrawing();
            CloseWindow();
            return 0;
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
