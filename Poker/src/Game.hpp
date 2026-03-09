#pragma once

#include <array>
#include <string>

#include "Card.hpp"
#include "CardSpriteAtlas.hpp"
#include "Deck.hpp"
#include "HandEvaluator.hpp"

class Game {
public:
    Game();
    ~Game() = default;

    void Update();
    void Draw() const;

private:
    enum class State {
        WaitingForRound,
        PlayerDraw,
        RoundOver,
        GameOver
    };

    void ResetGame();
    void StartRound();
    void FinishPlayerTurn();
    void DealerPlay();
    void ResolveRound();
    void ClampBet();

    void DrawLayout() const;
    void DrawSidebar() const;
    void DrawCenterBanner() const;
    void DrawCardVisual(const Card& card, int x, int y, bool hidden, bool held, int indexHint) const;
    void DrawCardBack(int x, int y) const;
    void DrawFallbackCard(const Card& card, int x, int y) const;
    void DrawEmptySlot(int x, int y) const;
    void DrawHand(const std::array<Card, 5>& hand, int y, bool hidden, const std::array<bool, 5>* heldFlags) const;
    void DrawHoldHints() const;

    std::array<bool, 5> GetDealerHoldDecision() const;
    bool TryHoldFourToFlush(std::array<bool, 5>& holdFlags) const;
    bool TryHoldFourToStraight(std::array<bool, 5>& holdFlags) const;
    int GetHandStartX() const;

    CardSpriteAtlas m_cardSprites;
    Deck m_deck;
    std::array<Card, 5> m_playerHand;
    std::array<Card, 5> m_dealerHand;
    std::array<bool, 5> m_playerHeld;

    State m_state;
    bool m_paused;
    bool m_dealerHidden;
    bool m_hasCards;

    int m_balance;
    int m_bet;
    int m_wins;
    int m_losses;
    int m_ties;

    std::string m_statusText;
    std::string m_roundResult;
    std::string m_playerRankText;
    std::string m_dealerRankText;
};
