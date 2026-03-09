#pragma once

#include <array>
#include <string>
#include <vector>

#include "Card.hpp"
#include "raylib.h"

class CardSpriteAtlas {
public:
    CardSpriteAtlas();
    ~CardSpriteAtlas();

    CardSpriteAtlas(const CardSpriteAtlas&) = delete;
    CardSpriteAtlas& operator=(const CardSpriteAtlas&) = delete;

    void Load();
    void Unload();

    const Texture2D* GetTexture(const Card& card) const;
    bool HasAnyTextures() const;
    const std::string& GetStatusText() const;

private:
    int GetIndex(const Card& card) const;
    std::vector<std::string> GetFileNameCandidates(const Card& card) const;

    std::array<Texture2D, 52> m_textures;
    std::array<bool, 52> m_loaded;
    bool m_anyLoaded;
    std::string m_statusText;
};
