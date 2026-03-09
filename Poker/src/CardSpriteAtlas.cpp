#include "CardSpriteAtlas.hpp"

#include <array>

namespace {
    std::vector<std::string> GetBasePaths() {
        return {"png/", "./png/", "../png/"};
    }

    std::vector<std::string> GetSuitPrefixes(Card::Suit suit) {
        switch (suit) {
        case Card::Suit::Clubs: return {"D", "C"};
        case Card::Suit::Diamonds: return {"H", "D"};
        case Card::Suit::Hearts: return {"C", "H"};
        case Card::Suit::Spades: return {"S"};
        }

        return {"X"};
    }
}

CardSpriteAtlas::CardSpriteAtlas()
    : m_anyLoaded(false), m_statusText("Card textures not loaded") {
    m_loaded.fill(false);
    for (Texture2D& texture : m_textures) {
        texture = Texture2D{};
    }
}

CardSpriteAtlas::~CardSpriteAtlas() {
    Unload();
}

void CardSpriteAtlas::Load() {
    Unload();

    int loadedCount = 0;
    for (int suit = 0; suit < 4; ++suit) {
        for (int rank = 2; rank <= 14; ++rank) {
            Card card(rank, static_cast<Card::Suit>(suit));
            const int index = GetIndex(card);
            const std::vector<std::string> names = GetFileNameCandidates(card);

            for (const std::string& name : names) {
                Texture2D texture = LoadTexture(name.c_str());
                if (texture.id != 0) {
                    m_textures[index] = texture;
                    m_loaded[index] = true;
                    ++loadedCount;
                    break;
                }
            }
        }
    }

    m_anyLoaded = loadedCount > 0;
    if (loadedCount == 52) {
        m_statusText = "Loaded all 52 card textures from png/";
    } else if (loadedCount > 0) {
        m_statusText = TextFormat("Loaded %i/52 card textures", loadedCount);
    } else {
        m_statusText = "No card textures found. Put png/ next to exe or one folder above it";
    }
}

void CardSpriteAtlas::Unload() {
    for (int i = 0; i < 52; ++i) {
        if (m_loaded[i] && m_textures[i].id != 0) {
            UnloadTexture(m_textures[i]);
        }
        m_textures[i] = Texture2D{};
        m_loaded[i] = false;
    }

    m_anyLoaded = false;
}

const Texture2D* CardSpriteAtlas::GetTexture(const Card& card) const {
    const int index = GetIndex(card);
    if (!m_loaded[index]) {
        return nullptr;
    }

    return &m_textures[index];
}

bool CardSpriteAtlas::HasAnyTextures() const {
    return m_anyLoaded;
}

const std::string& CardSpriteAtlas::GetStatusText() const {
    return m_statusText;
}

int CardSpriteAtlas::GetIndex(const Card& card) const {
    const int suitIndex = static_cast<int>(card.GetSuit());
    const int rankIndex = card.GetRank() - 2;
    return suitIndex * 13 + rankIndex;
}

std::vector<std::string> CardSpriteAtlas::GetFileNameCandidates(const Card& card) const {
    std::vector<std::string> candidates;
    const std::string rankText = card.ToAssetRankString();
    const std::vector<std::string> basePaths = GetBasePaths();
    const std::vector<std::string> suitPrefixes = GetSuitPrefixes(card.GetSuit());

    for (const std::string& basePath : basePaths) {
        for (const std::string& prefix : suitPrefixes) {
            candidates.push_back(basePath + prefix + rankText + ".png");
        }
    }

    return candidates;
}
