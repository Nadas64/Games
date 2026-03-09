#include "Board.hpp"

Board::Board()
{
    Clear();
}

void Board::Clear()
{
    for (auto& row : m_grid)
    {
        row.fill(0);
    }
}

bool Board::IsCellFree(int row, int col) const
{
    if (col < 0 || col >= BOARD_COLS || row >= BOARD_ROWS)
    {
        return false;
    }

    if (row < 0)
    {
        return true;
    }

    return m_grid[row][col] == 0;
}

bool Board::CanPlace(const Tetromino& piece, int rotationOffset, int rowOffset, int colOffset) const
{
    for (const CellPosition& cell : piece.GetCells(rotationOffset, rowOffset, colOffset))
    {
        if (!IsCellFree(cell.row, cell.col))
        {
            return false;
        }
    }

    return true;
}

bool Board::Place(const Tetromino& piece)
{
    bool overflowedAboveBoard = false;

    for (const CellPosition& cell : piece.GetCells())
    {
        if (cell.row < 0)
        {
            overflowedAboveBoard = true;
            continue;
        }

        if (cell.row >= 0 && cell.row < BOARD_ROWS && cell.col >= 0 && cell.col < BOARD_COLS)
        {
            m_grid[cell.row][cell.col] = piece.GetType() + 1;
        }
    }

    return overflowedAboveBoard;
}

int Board::ClearFullLines()
{
    int completed = 0;

    for (int row = BOARD_ROWS - 1; row >= 0; --row)
    {
        if (IsRowFull(row))
        {
            ClearRow(row);
            ++completed;
        }
        else if (completed > 0)
        {
            MoveRowDown(row, completed);
        }
    }

    return completed;
}

void Board::Draw(int offsetX, int offsetY, int cellSize) const
{
    for (int row = 0; row < BOARD_ROWS; ++row)
    {
        for (int col = 0; col < BOARD_COLS; ++col)
        {
            const int x = offsetX + col * cellSize;
            const int y = offsetY + row * cellSize;
            DrawRectangle(x, y, cellSize, cellSize, GetPieceColor(m_grid[row][col]));
            DrawRectangleLines(x, y, cellSize, cellSize, GetGridLineColor());
        }
    }
}

bool Board::IsRowFull(int row) const
{
    for (int col = 0; col < BOARD_COLS; ++col)
    {
        if (m_grid[row][col] == 0)
        {
            return false;
        }
    }

    return true;
}

void Board::ClearRow(int row)
{
    m_grid[row].fill(0);
}

void Board::MoveRowDown(int row, int amount)
{
    for (int col = 0; col < BOARD_COLS; ++col)
    {
        m_grid[row + amount][col] = m_grid[row][col];
        m_grid[row][col] = 0;
    }
}
