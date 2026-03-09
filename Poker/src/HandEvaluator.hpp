#pragma once

#include <array>
#include <string>
#include <vector>

#include "Card.hpp"

class HandEvaluator {
public:
    enum class Rank {
        HighCard = 0,
        OnePair,
        TwoPair,
        ThreeOfAKind,
        Straight,
        Flush,
        FullHouse,
        FourOfAKind,
        StraightFlush,
        RoyalFlush
    };

    struct HandValue {
        Rank rank = Rank::HighCard;
        std::vector<int> tiebreakers;
    };

    static HandValue Evaluate(const std::array<Card, 5>& hand);
    static int Compare(const HandValue& a, const HandValue& b);
    static std::string RankToString(Rank rank);

private:
    static bool IsFlush(const std::array<Card, 5>& hand);
    static bool IsStraight(const std::vector<int>& sortedRanksDesc, int& highCard);
};
