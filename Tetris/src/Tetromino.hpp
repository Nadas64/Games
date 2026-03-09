#pragma once

#include <array>
#include "Config.hpp"

class Tetromino
{
public:
    explicit Tetromino(int type = 0);

    int GetType() const;
    int GetRotation() const;
    int GetRow() const;
    int GetCol() const;

    void SetPosition(int newRow, int newCol);
    void Move(int rowOffset, int colOffset);
    void RotateClockwise();
    void RotateCounterClockwise();
    void Reset();

    std::array<CellPosition, 4> GetCells(int rotationOffset = 0, int rowOffset = 0, int colOffset = 0) const;

private:
    int m_type;
    int m_rotation;
    int m_row;
    int m_col;
};
