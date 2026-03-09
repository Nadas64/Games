#pragma once

#include <array>
#include "Tetromino.hpp"

class Board
{
public:
    Board();

    void Clear();
    bool IsCellFree(int row, int col) const;
    bool CanPlace(const Tetromino& piece, int rotationOffset = 0, int rowOffset = 0, int colOffset = 0) const;
    bool Place(const Tetromino& piece);
    int ClearFullLines();
    void Draw(int offsetX, int offsetY, int cellSize) const;

private:
    bool IsRowFull(int row) const;
    void ClearRow(int row);
    void MoveRowDown(int row, int amount);

    std::array<std::array<int, BOARD_COLS>, BOARD_ROWS> m_grid;
};
