#pragma once

namespace Config {
    constexpr int ScreenWidth = 1440;
    constexpr int ScreenHeight = 900;
    constexpr const char* WindowTitle = "Raylib Poker";

    constexpr int CardWidth = 120;
    constexpr int CardHeight = 174;
    constexpr int CardSpacing = 22;

    constexpr int TableMargin = 24;
    constexpr int SidebarWidth = 320;

    constexpr int DealerLabelY = 48;
    constexpr int DealerY = 88;
    constexpr int CenterBannerY = 344;
    constexpr int PlayerLabelY = 510;
    constexpr int PlayerY = 550;

    constexpr int InitialBalance = 1000;
    constexpr int MinBet = 10;
    constexpr int BetStep = 10;
}
