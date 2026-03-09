#include "Deck.hpp"

#include <algorithm>
#include <random>

Deck::Deck()
    : m_index(0) {
    ResetAndShuffle();
}

void Deck::ResetAndShuffle() {
    m_cards.clear();
    m_cards.reserve(52);

    for (int suit = 0; suit < 4; ++suit) {
        for (int rank = 2; rank <= 14; ++rank) {
            m_cards.emplace_back(rank, static_cast<Card::Suit>(suit));
        }
    }

    std::random_device rd;
    std::mt19937 rng(rd());
    std::shuffle(m_cards.begin(), m_cards.end(), rng);
    m_index = 0;
}

Card Deck::Draw() {
    if (m_index >= static_cast<int>(m_cards.size())) {
        ResetAndShuffle();
    }

    return m_cards[m_index++];
}

bool Deck::Empty() const {
    return m_index >= static_cast<int>(m_cards.size());
}
