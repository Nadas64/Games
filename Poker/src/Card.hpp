#pragma once

#include <string>

class Card {
public:
    enum class Suit {
        Clubs,
        Diamonds,
        Hearts,
        Spades
    };

    Card();
    Card(int rank, Suit suit);

    int GetRank() const;
    Suit GetSuit() const;

    std::string RankToString() const;
    std::string SuitToString() const;
    std::string ToShortString() const;
    std::string ToAssetRankString() const;
    bool IsRed() const;

private:
    int m_rank;
    Suit m_suit;
};
