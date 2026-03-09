#pragma once

#include <vector>
#include "Card.hpp"

class Deck {
public:
    Deck();

    void ResetAndShuffle();
    Card Draw();
    bool Empty() const;

private:
    std::vector<Card> m_cards;
    int m_index;
};
