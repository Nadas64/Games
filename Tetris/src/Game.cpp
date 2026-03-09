#include "Game.hpp"

#include <algorithm>
#include <string>

namespace
{
    constexpr float HORIZONTAL_INITIAL_DELAY = 0.14f;
    constexpr float HORIZONTAL_REPEAT_DELAY = 0.06f;
}

Game::Game()
    : m_currentPiece(0),
      m_nextPiece(0),
      m_score(0),
      m_totalLines(0),
      m_level(1),
      m_gameOver(false),
      m_paused(false),
      m_dropTimer(0.0f),
      m_horizontalHoldTimer(0.0f),
      m_horizontalRepeatTimer(0.0f),
      m_lastHorizontalDirection(0),
      m_rng(std::random_device{}())
{
    Reset();
}

void Game::Update()
{
    const float deltaTime = GetFrameTime();

    if (IsKeyPressed(KEY_R))
    {
        Reset();
        return;
    }

    if (IsKeyPressed(KEY_P) && !m_gameOver)
    {
        m_paused = !m_paused;
    }

    if (m_gameOver || m_paused)
    {
        return;
    }

    HandleInput();

    m_dropTimer += deltaTime;
    while (m_dropTimer >= GetDropInterval())
    {
        m_dropTimer -= GetDropInterval();
        if (!TryStepDown(IsKeyDown(KEY_DOWN)))
        {
            break;
        }
    }
}

void Game::Draw() const
{
    ClearBackground(GetBackgroundColor());
    m_board.Draw(BOARD_OFFSET_X, BOARD_OFFSET_Y, CELL_SIZE);
    DrawGhostPiece();
    DrawCurrentPiece();
    DrawPanel();
}

void Game::Reset()
{
    m_board.Clear();
    m_score = 0;
    m_totalLines = 0;
    m_level = 1;
    m_gameOver = false;
    m_paused = false;
    m_dropTimer = 0.0f;
    m_horizontalHoldTimer = 0.0f;
    m_horizontalRepeatTimer = 0.0f;
    m_lastHorizontalDirection = 0;
    m_bag.clear();

    m_currentPiece = Tetromino(GetNextTypeFromBag());
    m_nextPiece = Tetromino(GetNextTypeFromBag());

    if (!m_board.CanPlace(m_currentPiece))
    {
        m_gameOver = true;
    }
}

void Game::HandleInput()
{
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_X))
    {
        TryRotate(true);
    }

    if (IsKeyPressed(KEY_Z))
    {
        TryRotate(false);
    }

    if (IsKeyPressed(KEY_SPACE))
    {
        HardDrop();
        return;
    }

    int direction = 0;
    if (IsKeyDown(KEY_LEFT) && !IsKeyDown(KEY_RIGHT))
    {
        direction = -1;
    }
    else if (IsKeyDown(KEY_RIGHT) && !IsKeyDown(KEY_LEFT))
    {
        direction = 1;
    }

    const float deltaTime = GetFrameTime();

    if (direction != 0)
    {
        if (direction != m_lastHorizontalDirection)
        {
            TryMoveHorizontal(direction);
            m_horizontalHoldTimer = 0.0f;
            m_horizontalRepeatTimer = 0.0f;
        }
        else
        {
            m_horizontalHoldTimer += deltaTime;
            if (m_horizontalHoldTimer >= HORIZONTAL_INITIAL_DELAY)
            {
                m_horizontalRepeatTimer += deltaTime;
                while (m_horizontalRepeatTimer >= HORIZONTAL_REPEAT_DELAY)
                {
                    m_horizontalRepeatTimer -= HORIZONTAL_REPEAT_DELAY;
                    TryMoveHorizontal(direction);
                }
            }
        }
    }
    else
    {
        m_horizontalHoldTimer = 0.0f;
        m_horizontalRepeatTimer = 0.0f;
    }

    m_lastHorizontalDirection = direction;
}

void Game::TryMoveHorizontal(int direction)
{
    if (m_board.CanPlace(m_currentPiece, 0, 0, direction))
    {
        m_currentPiece.Move(0, direction);
    }
}

void Game::TryRotate(bool clockwise)
{
    const int rotationOffset = clockwise ? 1 : -1;
    static const CellPosition kicks[] = {
        {0, 0}, {0, -1}, {0, 1}, {-1, 0}, {1, 0}, {0, -2}, {0, 2}, {-2, 0}, {2, 0}
    };

    for (const CellPosition& kick : kicks)
    {
        if (m_board.CanPlace(m_currentPiece, rotationOffset, kick.row, kick.col))
        {
            if (clockwise)
            {
                m_currentPiece.RotateClockwise();
            }
            else
            {
                m_currentPiece.RotateCounterClockwise();
            }

            m_currentPiece.Move(kick.row, kick.col);
            return;
        }
    }
}

bool Game::TryStepDown(bool rewardSoftDrop)
{
    if (m_board.CanPlace(m_currentPiece, 0, 1, 0))
    {
        m_currentPiece.Move(1, 0);
        if (rewardSoftDrop)
        {
            m_score += 1;
        }
        return true;
    }

    LockCurrentPiece();
    return false;
}

void Game::HardDrop()
{
    int distance = 0;
    while (m_board.CanPlace(m_currentPiece, 0, 1, 0))
    {
        m_currentPiece.Move(1, 0);
        ++distance;
    }

    m_score += distance * 2;
    LockCurrentPiece();
}

void Game::LockCurrentPiece()
{
    if (m_board.Place(m_currentPiece))
    {
        m_gameOver = true;
        return;
    }

    const int cleared = m_board.ClearFullLines();
    UpdateScore(cleared);
    SpawnNextPiece();
}

void Game::SpawnNextPiece()
{
    m_currentPiece = m_nextPiece;
    m_currentPiece.Reset();
    m_nextPiece = Tetromino(GetNextTypeFromBag());
    m_dropTimer = 0.0f;

    if (!m_board.CanPlace(m_currentPiece))
    {
        m_gameOver = true;
    }
}

void Game::UpdateScore(int clearedLines)
{
    static const int lineScores[5] = {0, 100, 300, 500, 800};

    if (clearedLines < 0 || clearedLines > 4)
    {
        return;
    }

    m_totalLines += clearedLines;
    m_score += lineScores[clearedLines] * m_level;
    m_level = 1 + (m_totalLines / 10);
}

void Game::RefillBag()
{
    m_bag = {0, 1, 2, 3, 4, 5, 6};
    std::shuffle(m_bag.begin(), m_bag.end(), m_rng);
}

int Game::GetNextTypeFromBag()
{
    if (m_bag.empty())
    {
        RefillBag();
    }

    const int value = m_bag.back();
    m_bag.pop_back();
    return value;
}

float Game::GetDropInterval() const
{
    if (IsKeyDown(KEY_DOWN))
    {
        return 0.035f;
    }

    const float interval = 0.8f - static_cast<float>(m_level - 1) * 0.07f;
    return interval < 0.08f ? 0.08f : interval;
}

void Game::DrawCurrentPiece() const
{
    for (const CellPosition& cell : m_currentPiece.GetCells())
    {
        if (cell.row < 0)
        {
            continue;
        }

        const int x = BOARD_OFFSET_X + cell.col * CELL_SIZE;
        const int y = BOARD_OFFSET_Y + cell.row * CELL_SIZE;
        DrawRectangle(x, y, CELL_SIZE, CELL_SIZE, GetPieceColor(m_currentPiece.GetType() + 1));
        DrawRectangleLines(x, y, CELL_SIZE, CELL_SIZE, BLACK);
    }
}

void Game::DrawGhostPiece() const
{
    const int distance = GetGhostDropDistance();
    if (distance <= 0)
    {
        return;
    }

    Color ghostColor = GetPieceColor(m_currentPiece.GetType() + 1);
    ghostColor.a = 60;

    for (const CellPosition& cell : m_currentPiece.GetCells(0, distance, 0))
    {
        if (cell.row < 0)
        {
            continue;
        }

        const int x = BOARD_OFFSET_X + cell.col * CELL_SIZE;
        const int y = BOARD_OFFSET_Y + cell.row * CELL_SIZE;
        DrawRectangle(x, y, CELL_SIZE, CELL_SIZE, ghostColor);
        DrawRectangleLines(x, y, CELL_SIZE, CELL_SIZE, GetPieceColor(m_currentPiece.GetType() + 1));
    }
}

void Game::DrawNextPiecePreview() const
{
    const int previewCell = 24;
    const int startX = BOARD_COLS * CELL_SIZE + 55;
    const int startY = 130;

    auto cells = m_nextPiece.GetCells();
    int minRow = cells[0].row;
    int minCol = cells[0].col;

    for (const CellPosition& cell : cells)
    {
        if (cell.row < minRow)
        {
            minRow = cell.row;
        }
        if (cell.col < minCol)
        {
            minCol = cell.col;
        }
    }

    for (const CellPosition& cell : cells)
    {
        const int x = startX + (cell.col - minCol) * previewCell;
        const int y = startY + (cell.row - minRow) * previewCell;
        DrawRectangle(x, y, previewCell, previewCell, GetPieceColor(m_nextPiece.GetType() + 1));
        DrawRectangleLines(x, y, previewCell, previewCell, BLACK);
    }
}

void Game::DrawPanel() const
{
    const int panelX = BOARD_COLS * CELL_SIZE;
    DrawRectangle(panelX, 0, SIDE_PANEL_WIDTH, WINDOW_HEIGHT, GetPanelColor());
    DrawRectangleLines(panelX, 0, SIDE_PANEL_WIDTH, WINDOW_HEIGHT, GetGridLineColor());

    DrawText("TETRIS", panelX + 45, 24, 32, GetTextColor());
    DrawText("Next:", panelX + 25, 92, 24, GetTextColor());
    DrawNextPiecePreview();

    DrawText((std::string("Score: ") + std::to_string(m_score)).c_str(), panelX + 25, 260, 24, GetTextColor());
    DrawText((std::string("Lines: ") + std::to_string(m_totalLines)).c_str(), panelX + 25, 300, 24, GetTextColor());
    DrawText((std::string("Level: ") + std::to_string(m_level)).c_str(), panelX + 25, 340, 24, GetTextColor());

    DrawText("Controls:", panelX + 25, 410, 22, GetTextColor());
    DrawText("<- -> move", panelX + 25, 445, 20, LIGHTGRAY);
    DrawText("Down soft drop", panelX + 25, 470, 20, LIGHTGRAY);
    DrawText("Up / X rotate", panelX + 25, 495, 20, LIGHTGRAY);
    DrawText("Z rotate back", panelX + 25, 520, 20, LIGHTGRAY);
    DrawText("Space hard drop", panelX + 25, 545, 20, LIGHTGRAY);
    DrawText("P pause, R restart", panelX + 25, 570, 20, LIGHTGRAY);

    if (m_paused)
    {
        DrawText("PAUSED", panelX + 50, 210, 30, YELLOW);
    }

    if (m_gameOver)
    {
        DrawText("GAME OVER", panelX + 20, 210, 30, RED);
        DrawText("Press R", panelX + 55, 245, 24, ORANGE);
    }
}

int Game::GetGhostDropDistance() const
{
    int distance = 0;
    while (m_board.CanPlace(m_currentPiece, 0, distance + 1, 0))
    {
        ++distance;
    }

    return distance;
}
