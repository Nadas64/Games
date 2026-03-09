#include "Card.hpp"

Card::Card()
    : m_rank(2), m_suit(Suit::Clubs) {
}

Card::Card(int rank, Suit suit)
    : m_rank(rank), m_suit(suit) {
}

int Card::GetRank() const {
    return m_rank;
}

Card::Suit Card::GetSuit() const {
    return m_suit;
}

std::string Card::RankToString() const {
    switch (m_rank) {
    case 14: return "A";
    case 13: return "K";
    case 12: return "Q";
    case 11: return "J";
    default: return std::to_string(m_rank);
    }
}

std::string Card::SuitToString() const {
    switch (m_suit) {
    case Suit::Clubs: return "C";
    case Suit::Diamonds: return "D";
    case Suit::Hearts: return "H";
    case Suit::Spades: return "S";
    }

    return "?";
}

std::string Card::ToShortString() const {
    return RankToString() + SuitToString();
}

std::string Card::ToAssetRankString() const {
    return RankToString();
}

bool Card::IsRed() const {
    return m_suit == Suit::Diamonds || m_suit == Suit::Hearts;
}
