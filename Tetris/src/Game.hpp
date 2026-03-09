#pragma once

#include <random>
#include <vector>
#include "Board.hpp"

class Game
{
public:
    Game();

    void Update();
    void Draw() const;
    void Reset();

private:
    void HandleInput();
    void TryMoveHorizontal(int direction);
    void TryRotate(bool clockwise);
    bool TryStepDown(bool rewardSoftDrop);
    void HardDrop();
    void LockCurrentPiece();
    void SpawnNextPiece();
    void UpdateScore(int clearedLines);
    void RefillBag();
    int GetNextTypeFromBag();
    float GetDropInterval() const;
    void DrawCurrentPiece() const;
    void DrawGhostPiece() const;
    void DrawNextPiecePreview() const;
    void DrawPanel() const;
    int GetGhostDropDistance() const;

    Board m_board;
    Tetromino m_currentPiece;
    Tetromino m_nextPiece;

    int m_score;
    int m_totalLines;
    int m_level;
    bool m_gameOver;
    bool m_paused;

    float m_dropTimer;
    float m_horizontalHoldTimer;
    float m_horizontalRepeatTimer;
    int m_lastHorizontalDirection;

    std::mt19937 m_rng;
    std::vector<int> m_bag;
};
