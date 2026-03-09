#include "Game.hpp"

#include <algorithm>
#include <map>
#include <set>
#include <vector>

#include "Config.hpp"
#include "raylib.h"

namespace {
    Color BackgroundColor() { return Color{11, 42, 28, 255}; }
    Color TableColor() { return Color{18, 102, 63, 255}; }
    Color SidebarColor() { return Color{7, 29, 22, 255}; }
    Color PanelColor() { return Color{13, 55, 38, 255}; }
    Color AccentColor() { return Color{235, 198, 79, 255}; }
    Color SoftTextColor() { return Color{232, 236, 232, 255}; }
    Color MutedTextColor() { return Color{172, 189, 179, 255}; }
    Color HoldColor() { return Color{255, 214, 92, 255}; }
    Color WinColor() { return Color{102, 220, 126, 255}; }
    Color LoseColor() { return Color{226, 96, 96, 255}; }
    Color ShadowColor() { return Color{0, 0, 0, 90}; }
    Color HiddenBackColor() { return Color{28, 39, 44, 255}; }
    Color HiddenBackInnerColor() { return Color{38, 52, 58, 255}; }

    constexpr int kDealerAreaY = 44;
    constexpr int kDealerAreaHeight = 250;
    constexpr int kCenterAreaY = 320;
    constexpr int kCenterAreaHeight = 122;
    constexpr int kPlayerAreaY = 470;
    constexpr int kPlayerAreaHeight = 290;

    constexpr int GetHandWidth() {
        return 5 * Config::CardWidth + 4 * Config::CardSpacing;
    }

    int GetCenteredTextX(const std::string& text, int fontSize, int left, int width) {
        return left + (width - MeasureText(text.c_str(), fontSize)) / 2;
    }

    int CountSuit(const std::array<Card, 5>& hand, Card::Suit suit) {
        int count = 0;
        for (const Card& card : hand) {
            if (card.GetSuit() == suit) {
                ++count;
            }
        }
        return count;
    }

    std::vector<int> UniqueRanks(const std::array<Card, 5>& hand) {
        std::set<int> unique;
        for (const Card& card : hand) {
            unique.insert(card.GetRank());
        }

        return std::vector<int>(unique.begin(), unique.end());
    }

    Color ResultColorFromText(const std::string& text) {
        if (text == "You win") {
            return WinColor();
        }
        if (text == "Dealer wins") {
            return LoseColor();
        }
        return AccentColor();
    }
}

Game::Game()
    : m_state(State::WaitingForRound),
      m_paused(false),
      m_dealerHidden(true),
      m_hasCards(false),
      m_balance(Config::InitialBalance),
      m_bet(Config::MinBet),
      m_wins(0),
      m_losses(0),
      m_ties(0) {
    m_cardSprites.Load();
    ResetGame();
}

void Game::ResetGame() {
    m_state = State::WaitingForRound;
    m_paused = false;
    m_dealerHidden = true;
    m_hasCards = false;
    m_balance = Config::InitialBalance;
    m_bet = Config::MinBet;
    m_wins = 0;
    m_losses = 0;
    m_ties = 0;
    m_playerHeld.fill(false);
    m_statusText = "Up/Down bet   Enter deal   1-5 hold   P pause   R reset";
    m_roundResult.clear();
    m_playerRankText.clear();
    m_dealerRankText.clear();
}

int Game::GetHandStartX() const {
    const int playAreaX = Config::SidebarWidth;
    const int playAreaWidth = Config::ScreenWidth - playAreaX;
    return playAreaX + (playAreaWidth - GetHandWidth()) / 2;
}

void Game::ClampBet() {
    if (m_balance <= 0) {
        m_bet = 0;
        return;
    }

    if (m_bet < Config::MinBet) {
        m_bet = Config::MinBet;
    }

    if (m_bet > m_balance) {
        m_bet = m_balance;
    }
}

void Game::StartRound() {
    ClampBet();
    if (m_balance < m_bet || m_bet <= 0) {
        return;
    }

    m_balance -= m_bet;
    m_deck.ResetAndShuffle();
    m_playerHeld.fill(false);
    m_dealerHidden = true;
    m_hasCards = true;
    m_roundResult.clear();
    m_playerRankText.clear();
    m_dealerRankText.clear();

    for (int i = 0; i < 5; ++i) {
        m_playerHand[i] = m_deck.Draw();
        m_dealerHand[i] = m_deck.Draw();
    }

    m_state = State::PlayerDraw;
    m_statusText = "Choose cards with 1-5, then press Enter to draw";
}

std::array<bool, 5> Game::GetDealerHoldDecision() const {
    std::array<bool, 5> holdFlags{};
    HandEvaluator::HandValue handValue = HandEvaluator::Evaluate(m_dealerHand);

    if (handValue.rank == HandEvaluator::Rank::RoyalFlush ||
        handValue.rank == HandEvaluator::Rank::StraightFlush ||
        handValue.rank == HandEvaluator::Rank::Flush ||
        handValue.rank == HandEvaluator::Rank::Straight ||
        handValue.rank == HandEvaluator::Rank::FullHouse) {
        holdFlags.fill(true);
        return holdFlags;
    }

    std::map<int, int> counts;
    for (const Card& card : m_dealerHand) {
        counts[card.GetRank()]++;
    }

    if (handValue.rank == HandEvaluator::Rank::FourOfAKind ||
        handValue.rank == HandEvaluator::Rank::ThreeOfAKind ||
        handValue.rank == HandEvaluator::Rank::TwoPair ||
        handValue.rank == HandEvaluator::Rank::OnePair) {
        for (int i = 0; i < 5; ++i) {
            if (counts[m_dealerHand[i].GetRank()] >= 2) {
                holdFlags[i] = true;
            }
        }
        return holdFlags;
    }

    if (TryHoldFourToFlush(holdFlags)) {
        return holdFlags;
    }

    if (TryHoldFourToStraight(holdFlags)) {
        return holdFlags;
    }

    for (int i = 0; i < 5; ++i) {
        if (m_dealerHand[i].GetRank() >= 13) {
            holdFlags[i] = true;
        }
    }

    return holdFlags;
}

bool Game::TryHoldFourToFlush(std::array<bool, 5>& holdFlags) const {
    holdFlags.fill(false);

    for (int suit = 0; suit < 4; ++suit) {
        Card::Suit currentSuit = static_cast<Card::Suit>(suit);
        if (CountSuit(m_dealerHand, currentSuit) >= 4) {
            for (int i = 0; i < 5; ++i) {
                holdFlags[i] = m_dealerHand[i].GetSuit() == currentSuit;
            }
            return true;
        }
    }

    return false;
}

bool Game::TryHoldFourToStraight(std::array<bool, 5>& holdFlags) const {
    holdFlags.fill(false);

    std::vector<int> ranks = UniqueRanks(m_dealerHand);
    if (ranks.size() < 4) {
        return false;
    }

    std::vector<int> extended = ranks;
    if (std::find(extended.begin(), extended.end(), 14) != extended.end()) {
        extended.insert(extended.begin(), 1);
    }

    for (std::size_t i = 0; i + 3 < extended.size(); ++i) {
        bool straightish = true;
        for (std::size_t j = 1; j < 4; ++j) {
            if (extended[i + j] != extended[i] + static_cast<int>(j)) {
                straightish = false;
                break;
            }
        }

        if (straightish) {
            std::set<int> wanted = {
                extended[i], extended[i + 1], extended[i + 2], extended[i + 3]
            };

            for (int k = 0; k < 5; ++k) {
                const int rank = m_dealerHand[k].GetRank();
                if (wanted.count(rank) > 0 || (rank == 14 && wanted.count(1) > 0)) {
                    holdFlags[k] = true;
                }
            }
            return true;
        }
    }

    return false;
}

void Game::DealerPlay() {
    const std::array<bool, 5> dealerHeld = GetDealerHoldDecision();
    for (int i = 0; i < 5; ++i) {
        if (!dealerHeld[i]) {
            m_dealerHand[i] = m_deck.Draw();
        }
    }
}

void Game::ResolveRound() {
    const HandEvaluator::HandValue playerValue = HandEvaluator::Evaluate(m_playerHand);
    const HandEvaluator::HandValue dealerValue = HandEvaluator::Evaluate(m_dealerHand);

    m_playerRankText = HandEvaluator::RankToString(playerValue.rank);
    m_dealerRankText = HandEvaluator::RankToString(dealerValue.rank);

    const int result = HandEvaluator::Compare(playerValue, dealerValue);
    if (result > 0) {
        m_balance += m_bet * 2;
        ++m_wins;
        m_roundResult = "You win";
    } else if (result < 0) {
        ++m_losses;
        m_roundResult = "Dealer wins";
    } else {
        m_balance += m_bet;
        ++m_ties;
        m_roundResult = "Tie";
    }

    ClampBet();
    m_dealerHidden = false;

    if (m_balance <= 0) {
        m_state = State::GameOver;
        m_statusText = "No chips left. Press R to reset";
    } else {
        m_state = State::RoundOver;
        m_statusText = "Press Enter or N for next round";
    }
}

void Game::FinishPlayerTurn() {
    for (int i = 0; i < 5; ++i) {
        if (!m_playerHeld[i]) {
            m_playerHand[i] = m_deck.Draw();
        }
    }

    DealerPlay();
    ResolveRound();
}

void Game::Update() {
    if (IsKeyPressed(KEY_R)) {
        ResetGame();
        return;
    }

    if (IsKeyPressed(KEY_P)) {
        m_paused = !m_paused;
    }

    if (m_paused) {
        return;
    }

    if ((m_state == State::WaitingForRound || m_state == State::RoundOver) && m_balance > 0) {
        if (IsKeyPressed(KEY_UP)) {
            m_bet = std::min(m_balance, m_bet + Config::BetStep);
        }
        if (IsKeyPressed(KEY_DOWN)) {
            m_bet -= Config::BetStep;
            if (m_bet < Config::MinBet) {
                m_bet = Config::MinBet;
            }
            if (m_bet > m_balance) {
                m_bet = m_balance;
            }
        }
    }

    switch (m_state) {
    case State::WaitingForRound:
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_N)) {
            StartRound();
        }
        break;

    case State::PlayerDraw:
        if (IsKeyPressed(KEY_ONE)) m_playerHeld[0] = !m_playerHeld[0];
        if (IsKeyPressed(KEY_TWO)) m_playerHeld[1] = !m_playerHeld[1];
        if (IsKeyPressed(KEY_THREE)) m_playerHeld[2] = !m_playerHeld[2];
        if (IsKeyPressed(KEY_FOUR)) m_playerHeld[3] = !m_playerHeld[3];
        if (IsKeyPressed(KEY_FIVE)) m_playerHeld[4] = !m_playerHeld[4];

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            FinishPlayerTurn();
        }
        break;

    case State::RoundOver:
        if (IsKeyPressed(KEY_N) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            StartRound();
        }
        break;

    case State::GameOver:
        break;
    }
}

void Game::DrawLayout() const {
    ClearBackground(BackgroundColor());

    DrawRectangle(0, 0, Config::ScreenWidth, Config::ScreenHeight, BackgroundColor());
    DrawRectangle(Config::TableMargin, Config::TableMargin,
                  Config::ScreenWidth - Config::TableMargin * 2,
                  Config::ScreenHeight - Config::TableMargin * 2,
                  TableColor());

    DrawRectangle(24, 24, Config::SidebarWidth - 36, Config::ScreenHeight - 48, SidebarColor());

    const int mainX = Config::SidebarWidth + 12;
    const int mainW = Config::ScreenWidth - mainX - 24;

    DrawRectangle(mainX, kDealerAreaY, mainW, kDealerAreaHeight, Fade(PanelColor(), 0.72f));
    DrawRectangle(mainX, kCenterAreaY, mainW, kCenterAreaHeight, Fade(SidebarColor(), 0.9f));
    DrawRectangle(mainX, kPlayerAreaY, mainW, kPlayerAreaHeight, Fade(PanelColor(), 0.58f));
}

void Game::DrawSidebar() const {
    const int x = 44;
    int y = 40;

    DrawText("POKER", x, y, 42, AccentColor());
    y += 76;

    DrawText("BALANCE", x, y, 18, MutedTextColor());
    y += 24;
    DrawText(TextFormat("%i", m_balance), x, y, 38, SoftTextColor());
    y += 56;

    DrawText("BET", x, y, 18, MutedTextColor());
    y += 24;
    DrawText(TextFormat("%i", m_bet), x, y, 38, AccentColor());
    y += 64;

    DrawText("SCORE", x, y, 18, MutedTextColor());
    y += 26;
    DrawText(TextFormat("Wins   %i", m_wins), x, y, 24, SoftTextColor());
    y += 30;
    DrawText(TextFormat("Losses %i", m_losses), x, y, 24, SoftTextColor());
    y += 30;
    DrawText(TextFormat("Ties   %i", m_ties), x, y, 24, SoftTextColor());
    y += 60;

    DrawText("CONTROLS", x, y, 18, MutedTextColor());
    y += 26;
    DrawText("UP / DOWN  Bet", x, y, 20, SoftTextColor());
    y += 26;
    DrawText("ENTER      Deal / Draw", x, y, 20, SoftTextColor());
    y += 26;
    DrawText("1 - 5      Hold cards", x, y, 20, SoftTextColor());
    y += 26;
    DrawText("N          Next round", x, y, 20, SoftTextColor());
    y += 26;
    DrawText("P          Pause", x, y, 20, SoftTextColor());
    y += 26;
    DrawText("R          Reset", x, y, 20, SoftTextColor());
    y += 60;

    DrawText("5-CARD DRAW", x, y, 22, AccentColor());
    y += 28;
    DrawText("Win = 2x bet", x, y, 22, SoftTextColor());
    y += 26;
    DrawText("Tie = bet returned", x, y, 22, SoftTextColor());
}

void Game::DrawCenterBanner() const {
    const int x = Config::SidebarWidth + 12;
    const int w = Config::ScreenWidth - x - 24;

    std::string headline;
    Color headlineColor = AccentColor();

    switch (m_state) {
    case State::WaitingForRound:
        headline = "Press Enter to deal";
        break;
    case State::PlayerDraw:
        headline = "Choose cards to hold";
        break;
    case State::RoundOver:
        headline = m_roundResult.empty() ? "Round over" : m_roundResult;
        headlineColor = ResultColorFromText(m_roundResult);
        break;
    case State::GameOver:
        headline = "Game over";
        headlineColor = LoseColor();
        break;
    }

    const int headlineX = GetCenteredTextX(headline, 34, x, w);
    DrawText(headline.c_str(), headlineX, kCenterAreaY + 18, 34, headlineColor);

    std::string detail = m_statusText;
    if (!m_playerRankText.empty() && !m_dealerHidden) {
        detail += "   |   Player: " + m_playerRankText + "   Dealer: " + m_dealerRankText;
    } else if (!m_playerRankText.empty()) {
        detail += "   |   Player: " + m_playerRankText;
    }

    const int detailX = GetCenteredTextX(detail, 22, x, w);
    DrawText(detail.c_str(), detailX, kCenterAreaY + 72, 22, SoftTextColor());
}

void Game::DrawCardBack(int x, int y) const {
    DrawRectangle(x + 6, y + 8, Config::CardWidth, Config::CardHeight, ShadowColor());
    DrawRectangle(x, y, Config::CardWidth, Config::CardHeight, HiddenBackColor());
    DrawRectangle(x + 8, y + 8, Config::CardWidth - 16, Config::CardHeight - 16, HiddenBackInnerColor());
    DrawRectangleLines(x, y, Config::CardWidth, Config::CardHeight, Fade(SoftTextColor(), 0.55f));
}

void Game::DrawFallbackCard(const Card& card, int x, int y) const {
    DrawRectangle(x + 6, y + 8, Config::CardWidth, Config::CardHeight, ShadowColor());
    DrawRectangle(x, y, Config::CardWidth, Config::CardHeight, RAYWHITE);
    DrawRectangleLines(x, y, Config::CardWidth, Config::CardHeight, BLACK);

    const Color textColor = card.IsRed() ? RED : BLACK;
    DrawText(card.RankToString().c_str(), x + 10, y + 10, 32, textColor);
    DrawText(card.SuitToString().c_str(), x + 12, y + 48, 30, textColor);
    DrawText(card.ToShortString().c_str(), x + 22, y + 118, 30, textColor);
}

void Game::DrawEmptySlot(int x, int y) const {
    DrawRectangle(x, y, Config::CardWidth, Config::CardHeight, Fade(BackgroundColor(), 0.28f));
    DrawRectangleLines(x, y, Config::CardWidth, Config::CardHeight, Fade(SoftTextColor(), 0.35f));
}

void Game::DrawCardVisual(const Card& card, int x, int y, bool hidden, bool held, int indexHint) const {
    if (hidden) {
        DrawCardBack(x, y);
    } else {
        DrawRectangle(x + 6, y + 8, Config::CardWidth, Config::CardHeight, ShadowColor());

        const Texture2D* texture = m_cardSprites.GetTexture(card);
        if (texture != nullptr) {
            Rectangle source{0.0f, 0.0f, static_cast<float>(texture->width), static_cast<float>(texture->height)};
            Rectangle target{static_cast<float>(x), static_cast<float>(y),
                             static_cast<float>(Config::CardWidth), static_cast<float>(Config::CardHeight)};
            DrawTexturePro(*texture, source, target, Vector2{0.0f, 0.0f}, 0.0f, RAYWHITE);
            DrawRectangleLines(x, y, Config::CardWidth, Config::CardHeight, Fade(BLACK, 0.65f));
        } else {
            DrawFallbackCard(card, x, y);
        }
    }

    if (held) {
        DrawRectangle(x, y - 28, Config::CardWidth, 22, HoldColor());
        DrawText("HELD", x + 32, y - 24, 18, BackgroundColor());
        DrawRectangleLines(x - 3, y - 3, Config::CardWidth + 6, Config::CardHeight + 6, HoldColor());
    }

    if (!hidden) {
        DrawText(TextFormat("[%i]", indexHint + 1), x + 42, y + Config::CardHeight + 12, 20, SoftTextColor());
    }
}

void Game::DrawHand(const std::array<Card, 5>& hand, int y, bool hidden, const std::array<bool, 5>* heldFlags) const {
    const int startX = GetHandStartX();

    for (int i = 0; i < 5; ++i) {
        const int x = startX + i * (Config::CardWidth + Config::CardSpacing);
        if (!m_hasCards) {
            DrawEmptySlot(x, y);
            continue;
        }

        const bool held = heldFlags != nullptr ? (*heldFlags)[i] : false;
        DrawCardVisual(hand[i], x, y, hidden, held, i);
    }
}

void Game::DrawHoldHints() const {
}

void Game::Draw() const {
    DrawLayout();
    DrawSidebar();
    DrawCenterBanner();

    const int startX = GetHandStartX();
    const int mainLeft = Config::SidebarWidth + 12;
    const int mainWidth = Config::ScreenWidth - mainLeft - 24;

    std::string dealerLabel = "DEALER";
    if (!m_dealerRankText.empty() && !m_dealerHidden) {
        dealerLabel += "  -  " + m_dealerRankText;
    }
    DrawText(dealerLabel.c_str(), GetCenteredTextX(dealerLabel, 28, mainLeft, mainWidth), 64, 28, SoftTextColor());
    DrawHand(m_dealerHand, 98, m_dealerHidden, nullptr);

    std::string playerLabel = "PLAYER";
    if (!m_playerRankText.empty()) {
        playerLabel += "  -  " + m_playerRankText;
    }
    DrawText(playerLabel.c_str(), GetCenteredTextX(playerLabel, 28, mainLeft, mainWidth), 504, 28, SoftTextColor());
    DrawHand(m_playerHand, 542, false, m_state == State::PlayerDraw ? &m_playerHeld : nullptr);

    if (m_state == State::PlayerDraw) {
        const char* holdInfo = "Tap 1-5 to hold selected cards";
        DrawText(holdInfo, GetCenteredTextX(holdInfo, 22, mainLeft, mainWidth), 782, 22, MutedTextColor());
    }

    if (m_paused) {
        DrawRectangle(0, 0, Config::ScreenWidth, Config::ScreenHeight, Fade(BLACK, 0.45f));
        DrawRectangle(490, 320, 460, 140, SidebarColor());
        DrawText("PAUSED", 640, 350, 40, RAYWHITE);
        DrawText("Press P to continue", 575, 400, 24, SoftTextColor());
    }
}
