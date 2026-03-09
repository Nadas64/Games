#include "Tetromino.hpp"

namespace
{
    using RotationData = std::array<CellPosition, 4>;
    using PieceData = std::array<RotationData, 4>;

    constexpr std::array<PieceData, 7> SHAPES = {{
        {{{{{1, 0}, {1, 1}, {1, 2}, {1, 3}}}, {{{0, 2}, {1, 2}, {2, 2}, {3, 2}}}, {{{2, 0}, {2, 1}, {2, 2}, {2, 3}}}, {{{0, 1}, {1, 1}, {2, 1}, {3, 1}}}}},
        {{{{{0, 0}, {1, 0}, {1, 1}, {1, 2}}}, {{{0, 1}, {0, 2}, {1, 1}, {2, 1}}}, {{{1, 0}, {1, 1}, {1, 2}, {2, 2}}}, {{{0, 1}, {1, 1}, {2, 0}, {2, 1}}}}},
        {{{{{0, 2}, {1, 0}, {1, 1}, {1, 2}}}, {{{0, 1}, {1, 1}, {2, 1}, {2, 2}}}, {{{1, 0}, {1, 1}, {1, 2}, {2, 0}}}, {{{0, 0}, {0, 1}, {1, 1}, {2, 1}}}}},
        {{{{{0, 1}, {0, 2}, {1, 1}, {1, 2}}}, {{{0, 1}, {0, 2}, {1, 1}, {1, 2}}}, {{{0, 1}, {0, 2}, {1, 1}, {1, 2}}}, {{{0, 1}, {0, 2}, {1, 1}, {1, 2}}}}},
        {{{{{0, 1}, {0, 2}, {1, 0}, {1, 1}}}, {{{0, 1}, {1, 1}, {1, 2}, {2, 2}}}, {{{1, 1}, {1, 2}, {2, 0}, {2, 1}}}, {{{0, 0}, {1, 0}, {1, 1}, {2, 1}}}}},
        {{{{{0, 1}, {1, 0}, {1, 1}, {1, 2}}}, {{{0, 1}, {1, 1}, {1, 2}, {2, 1}}}, {{{1, 0}, {1, 1}, {1, 2}, {2, 1}}}, {{{0, 1}, {1, 0}, {1, 1}, {2, 1}}}}},
        {{{{{0, 0}, {0, 1}, {1, 1}, {1, 2}}}, {{{0, 2}, {1, 1}, {1, 2}, {2, 1}}}, {{{1, 0}, {1, 1}, {2, 1}, {2, 2}}}, {{{0, 1}, {1, 0}, {1, 1}, {2, 0}}}}}
    }};
}

Tetromino::Tetromino(int type) : m_type(type), m_rotation(0), m_row(0), m_col(3) {}

int Tetromino::GetType() const { return m_type; }
int Tetromino::GetRotation() const { return m_rotation; }
int Tetromino::GetRow() const { return m_row; }
int Tetromino::GetCol() const { return m_col; }

void Tetromino::SetPosition(int newRow, int newCol)
{
    m_row = newRow;
    m_col = newCol;
}

void Tetromino::Move(int rowOffset, int colOffset)
{
    m_row += rowOffset;
    m_col += colOffset;
}

void Tetromino::RotateClockwise() { m_rotation = (m_rotation + 1) % 4; }
void Tetromino::RotateCounterClockwise() { m_rotation = (m_rotation + 3) % 4; }

void Tetromino::Reset()
{
    m_rotation = 0;
    m_row = 0;
    m_col = 3;
}

std::array<CellPosition, 4> Tetromino::GetCells(int rotationOffset, int rowOffset, int colOffset) const
{
    const int rotation = (m_rotation + rotationOffset + 4) % 4;
    std::array<CellPosition, 4> cells = SHAPES[m_type][rotation];

    for (CellPosition& cell : cells)
    {
        cell.row += m_row + rowOffset;
        cell.col += m_col + colOffset;
    }

    return cells;
}
