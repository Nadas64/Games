#include "HandEvaluator.hpp"

#include <algorithm>
#include <map>

namespace {
    std::vector<int> GetRanksSortedDesc(const std::array<Card, 5>& hand) {
        std::vector<int> ranks;
        ranks.reserve(5);

        for (const Card& card : hand) {
            ranks.push_back(card.GetRank());
        }

        std::sort(ranks.begin(), ranks.end(), std::greater<int>());
        return ranks;
    }
}

HandEvaluator::HandValue HandEvaluator::Evaluate(const std::array<Card, 5>& hand) {
    HandValue value;

    std::vector<int> ranksDesc = GetRanksSortedDesc(hand);
    bool flush = IsFlush(hand);
    int straightHigh = 0;
    bool straight = IsStraight(ranksDesc, straightHigh);

    std::map<int, int, std::greater<int>> rankCounts;
    for (const Card& card : hand) {
        rankCounts[card.GetRank()]++;
    }

    std::vector<std::pair<int, int>> countRankPairs;
    countRankPairs.reserve(rankCounts.size());
    for (const auto& [rank, count] : rankCounts) {
        countRankPairs.push_back({count, rank});
    }

    std::sort(countRankPairs.begin(), countRankPairs.end(), [](const auto& a, const auto& b) {
        if (a.first != b.first) {
            return a.first > b.first;
        }
        return a.second > b.second;
    });

    if (flush && straight && straightHigh == 14 &&
        std::find(ranksDesc.begin(), ranksDesc.end(), 10) != ranksDesc.end()) {
        value.rank = Rank::RoyalFlush;
        return value;
    }

    if (flush && straight) {
        value.rank = Rank::StraightFlush;
        value.tiebreakers = {straightHigh};
        return value;
    }

    if (countRankPairs[0].first == 4) {
        value.rank = Rank::FourOfAKind;
        int quad = countRankPairs[0].second;
        int kicker = countRankPairs[1].second;
        value.tiebreakers = {quad, kicker};
        return value;
    }

    if (countRankPairs[0].first == 3 && countRankPairs[1].first == 2) {
        value.rank = Rank::FullHouse;
        value.tiebreakers = {countRankPairs[0].second, countRankPairs[1].second};
        return value;
    }

    if (flush) {
        value.rank = Rank::Flush;
        value.tiebreakers = ranksDesc;
        return value;
    }

    if (straight) {
        value.rank = Rank::Straight;
        value.tiebreakers = {straightHigh};
        return value;
    }

    if (countRankPairs[0].first == 3) {
        value.rank = Rank::ThreeOfAKind;
        value.tiebreakers.push_back(countRankPairs[0].second);
        for (const auto& [count, rank] : countRankPairs) {
            if (count == 1) {
                value.tiebreakers.push_back(rank);
            }
        }
        return value;
    }

    if (countRankPairs[0].first == 2 && countRankPairs[1].first == 2) {
        value.rank = Rank::TwoPair;
        int highPair = std::max(countRankPairs[0].second, countRankPairs[1].second);
        int lowPair = std::min(countRankPairs[0].second, countRankPairs[1].second);
        int kicker = countRankPairs[2].second;
        value.tiebreakers = {highPair, lowPair, kicker};
        return value;
    }

    if (countRankPairs[0].first == 2) {
        value.rank = Rank::OnePair;
        value.tiebreakers.push_back(countRankPairs[0].second);
        for (const auto& [count, rank] : countRankPairs) {
            if (count == 1) {
                value.tiebreakers.push_back(rank);
            }
        }
        return value;
    }

    value.rank = Rank::HighCard;
    value.tiebreakers = ranksDesc;
    return value;
}

int HandEvaluator::Compare(const HandValue& a, const HandValue& b) {
    if (a.rank != b.rank) {
        return static_cast<int>(a.rank) > static_cast<int>(b.rank) ? 1 : -1;
    }

    const std::size_t count = std::min(a.tiebreakers.size(), b.tiebreakers.size());
    for (std::size_t i = 0; i < count; ++i) {
        if (a.tiebreakers[i] != b.tiebreakers[i]) {
            return a.tiebreakers[i] > b.tiebreakers[i] ? 1 : -1;
        }
    }

    if (a.tiebreakers.size() == b.tiebreakers.size()) {
        return 0;
    }

    return a.tiebreakers.size() > b.tiebreakers.size() ? 1 : -1;
}

std::string HandEvaluator::RankToString(Rank rank) {
    switch (rank) {
    case Rank::HighCard: return "High Card";
    case Rank::OnePair: return "One Pair";
    case Rank::TwoPair: return "Two Pair";
    case Rank::ThreeOfAKind: return "Three of a Kind";
    case Rank::Straight: return "Straight";
    case Rank::Flush: return "Flush";
    case Rank::FullHouse: return "Full House";
    case Rank::FourOfAKind: return "Four of a Kind";
    case Rank::StraightFlush: return "Straight Flush";
    case Rank::RoyalFlush: return "Royal Flush";
    }

    return "Unknown";
}

bool HandEvaluator::IsFlush(const std::array<Card, 5>& hand) {
    Card::Suit suit = hand[0].GetSuit();
    for (int i = 1; i < 5; ++i) {
        if (hand[i].GetSuit() != suit) {
            return false;
        }
    }
    return true;
}

bool HandEvaluator::IsStraight(const std::vector<int>& sortedRanksDesc, int& highCard) {
    std::vector<int> ranks = sortedRanksDesc;
    std::sort(ranks.begin(), ranks.end());
    ranks.erase(std::unique(ranks.begin(), ranks.end()), ranks.end());

    if (ranks.size() != 5) {
        return false;
    }

    bool consecutive = true;
    for (int i = 1; i < 5; ++i) {
        if (ranks[i] != ranks[0] + i) {
            consecutive = false;
            break;
        }
    }

    if (consecutive) {
        highCard = ranks.back();
        return true;
    }

    if (ranks[0] == 2 && ranks[1] == 3 && ranks[2] == 4 && ranks[3] == 5 && ranks[4] == 14) {
        highCard = 5;
        return true;
    }

    return false;
}
