#pragma once

#include "raylib.h"

constexpr int BOARD_ROWS = 20;
constexpr int BOARD_COLS = 10;
constexpr int CELL_SIZE = 30;
constexpr int SIDE_PANEL_WIDTH = 220;
constexpr int WINDOW_WIDTH = BOARD_COLS * CELL_SIZE + SIDE_PANEL_WIDTH;
constexpr int WINDOW_HEIGHT = BOARD_ROWS * CELL_SIZE;

constexpr int BOARD_OFFSET_X = 0;
constexpr int BOARD_OFFSET_Y = 0;

struct CellPosition
{
    int row;
    int col;
};

inline Color GetPieceColor(int value)
{
    static const Color colors[8] = {
        {18, 18, 18, 255},
        {0, 240, 240, 255},
        {0, 102, 255, 255},
        {255, 153, 0, 255},
        {255, 230, 0, 255},
        {0, 204, 102, 255},
        {180, 0, 255, 255},
        {255, 51, 51, 255}
    };

    if (value < 0 || value > 7)
    {
        return colors[0];
    }

    return colors[value];
}

inline Color GetBackgroundColor()
{
    return {12, 12, 18, 255};
}

inline Color GetPanelColor()
{
    return {24, 24, 34, 255};
}

inline Color GetGridLineColor()
{
    return {32, 32, 48, 255};
}

inline Color GetTextColor()
{
    return RAYWHITE;
}
