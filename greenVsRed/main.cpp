#include "raylib.h"
#include <vector>
#include <array>
#include <random>
#include <ctime>
#include <cmath>
#include <algorithm>

constexpr int WINDOW_SIZE = 640;
constexpr int CELL_SIZE   = 4;
constexpr int COLS        = WINDOW_SIZE / CELL_SIZE;
constexpr int ROWS        = WINDOW_SIZE / CELL_SIZE;

static std::mt19937 rng((unsigned)std::time(nullptr));

static int RandInt(int a, int b)
{
    std::uniform_int_distribution<int> dist(a, b);
    return dist(rng);
}

static float RandFloat(float a, float b)
{
    std::uniform_real_distribution<float> dist(a, b);
    return dist(rng);
}

static bool Chance(float p)
{
    return RandFloat(0.0f, 1.0f) < p;
}

static int Idx(int x, int y) { return y * COLS + x; }

static bool InBounds(int x, int y)
{
    return x >= 0 && x < COLS && y >= 0 && y < ROWS;
}

static Vector2 NormalizeSafe(Vector2 v)
{
    float len = std::sqrt(v.x * v.x + v.y * v.y);
    if (len < 0.0001f) return {0.0f, 0.0f};
    return {v.x / len, v.y / len};
}

static Vector2 Limit(Vector2 v, float maxLen)
{
    float len = std::sqrt(v.x * v.x + v.y * v.y);
    if (len > maxLen && len > 0.0001f)
    {
        v.x = v.x / len * maxLen;
        v.y = v.y / len * maxLen;
    }
    return v;
}

struct TerritorySim
{
    enum Cell : unsigned char
    {
        Empty,
        Green,
        Red,
        Blue,
        Rock
    };

    std::vector<Cell> grid;
    std::vector<Cell> next;
    double meteorTimer = 0.0;
    int greenCount = 0;
    int redCount = 0;
    int blueCount = 0;

    TerritorySim() : grid(ROWS * COLS, Empty), next(ROWS * COLS, Empty)
    {
        Reset();
    }

    void SeedDisc(int cx, int cy, int r, Cell t)
    {
        for (int y = cy - r; y <= cy + r; ++y)
        {
            for (int x = cx - r; x <= cx + r; ++x)
            {
                if (!InBounds(x, y)) continue;
                int dx = x - cx;
                int dy = y - cy;
                if (dx * dx + dy * dy <= r * r)
                    grid[Idx(x, y)] = t;
            }
        }
    }

    int CountAround(int x, int y, Cell t) const
    {
        int count = 0;
        for (int oy = -1; oy <= 1; ++oy)
        {
            for (int ox = -1; ox <= 1; ++ox)
            {
                if (ox == 0 && oy == 0) continue;
                int nx = x + ox;
                int ny = y + oy;
                if (InBounds(nx, ny) && grid[Idx(nx, ny)] == t)
                    ++count;
            }
        }
        return count;
    }

    void Recount()
    {
        greenCount = redCount = blueCount = 0;
        for (Cell c : grid)
        {
            if (c == Green) ++greenCount;
            else if (c == Red) ++redCount;
            else if (c == Blue) ++blueCount;
        }
    }

    void Impact()
    {
        int cx = RandInt(20, COLS - 21);
        int cy = RandInt(20, ROWS - 21);
        int r = RandInt(5, 12);

        for (int y = cy - r; y <= cy + r; ++y)
        {
            for (int x = cx - r; x <= cx + r; ++x)
            {
                if (!InBounds(x, y)) continue;
                int dx = x - cx;
                int dy = y - cy;
                if (dx * dx + dy * dy <= r * r)
                    grid[Idx(x, y)] = Chance(0.18f) ? Rock : Empty;
            }
        }
    }

    void Reset()
    {
        std::fill(grid.begin(), grid.end(), Empty);

        for (int i = 0; i < (ROWS * COLS) / 45; ++i)
            grid[Idx(RandInt(0, COLS - 1), RandInt(0, ROWS - 1))] = Rock;

        SeedDisc(10, 10, 4, Green);
        SeedDisc(COLS - 11, ROWS - 11, 4, Red);
        SeedDisc(COLS / 2, 10, 4, Blue);

        meteorTimer = 0.0;
        Recount();
    }

    void Update(float dt)
    {
        meteorTimer += dt;
        if (meteorTimer > 7.0)
        {
            Impact();
            meteorTimer = 0.0;
        }

        next = grid;

        for (int y = 0; y < ROWS; ++y)
        {
            for (int x = 0; x < COLS; ++x)
            {
                int id = Idx(x, y);
                Cell c = grid[id];

                if (c == Rock) continue;

                if (c == Empty)
                {
                    int infGreen = CountAround(x, y, Green);
                    int infRed   = CountAround(x, y, Red);
                    int infBlue  = CountAround(x, y, Blue);
                    int best = std::max({infGreen, infRed, infBlue});

                    if (best > 0)
                    {
                        std::array<Cell, 3> winners{};
                        int wc = 0;
                        if (infGreen == best) winners[wc++] = Green;
                        if (infRed   == best) winners[wc++] = Red;
                        if (infBlue  == best) winners[wc++] = Blue;

                        Cell winner = winners[RandInt(0, wc - 1)];
                        float spreadChance = 0.01f + 0.045f * best;

                        if (Chance(spreadChance))
                            next[id] = winner;
                    }
                }
                else
                {
                    int own = CountAround(x, y, c);
                    int enemy = 0;
                    if (c != Green) enemy += CountAround(x, y, Green);
                    if (c != Red)   enemy += CountAround(x, y, Red);
                    if (c != Blue)  enemy += CountAround(x, y, Blue);

                    if (enemy >= own + 3 && Chance(0.05f))
                        next[id] = Empty;
                    else if (Chance(0.0007f))
                        next[id] = Rock;
                }
            }
        }

        grid.swap(next);
        Recount();
    }

    void Draw() const
    {
        for (int y = 0; y < ROWS; ++y)
        {
            for (int x = 0; x < COLS; ++x)
            {
                Color col = BLACK;
                switch (grid[Idx(x, y)])
                {
                    case Green: col = GREEN; break;
                    case Red:   col = RED; break;
                    case Blue:  col = SKYBLUE; break;
                    case Rock:  col = GRAY; break;
                    default:    col = BLACK; break;
                }
                DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, col);
            }
        }

        DrawRectangle(8, 8, 210, 76, Fade(BLACK, 0.65f));
        DrawText("1: Territory War", 16, 14, 18, RAYWHITE);
        DrawText(TextFormat("Green: %d", greenCount), 16, 36, 16, GREEN);
        DrawText(TextFormat("Red:   %d", redCount),   16, 54, 16, RED);
        DrawText(TextFormat("Blue:  %d", blueCount),  16, 72, 16, SKYBLUE);
    }
};

struct GardenSim
{
    enum Cell : unsigned char
    {
        Soil,
        Water,
        Grass,
        Flower,
        Tree,
        Weed,
        Stone
    };

    struct Bee
    {
        Vector2 pos{};
        Vector2 vel{};
        bool pollen = false;
    };

    std::vector<Cell> grid;
    std::vector<Cell> next;
    std::vector<Bee> bees;
    double rainTimer = 0.0;
    double seasonTimer = 0.0;
    int season = 0; // 0 spring, 1 summer, 2 autumn, 3 winter

    int grassCount = 0;
    int flowerCount = 0;
    int treeCount = 0;
    int weedCount = 0;

    GardenSim() : grid(ROWS * COLS, Soil), next(ROWS * COLS, Soil)
    {
        Reset();
    }

    const char* SeasonName() const
    {
        switch (season)
        {
            case 0: return "Spring";
            case 1: return "Summer";
            case 2: return "Autumn";
            default: return "Winter";
        }
    }

    Color BackgroundColor() const
    {
        switch (season)
        {
            case 0: return Color{18, 24, 18, 255};
            case 1: return Color{20, 20, 14, 255};
            case 2: return Color{26, 18, 10, 255};
            default:return Color{18, 22, 28, 255};
        }
    }

    int CountAround(int x, int y, Cell t) const
    {
        int count = 0;
        for (int oy = -1; oy <= 1; ++oy)
        {
            for (int ox = -1; ox <= 1; ++ox)
            {
                if (ox == 0 && oy == 0) continue;
                int nx = x + ox;
                int ny = y + oy;
                if (InBounds(nx, ny) && grid[Idx(nx, ny)] == t)
                    ++count;
            }
        }
        return count;
    }

    void Recount()
    {
        grassCount = flowerCount = treeCount = weedCount = 0;
        for (Cell c : grid)
        {
            if (c == Grass) ++grassCount;
            else if (c == Flower) ++flowerCount;
            else if (c == Tree) ++treeCount;
            else if (c == Weed) ++weedCount;
        }
    }

    void MakePond(int cx, int cy, int rx, int ry)
    {
        for (int y = cy - ry; y <= cy + ry; ++y)
        {
            for (int x = cx - rx; x <= cx + rx; ++x)
            {
                if (!InBounds(x, y)) continue;
                float dx = float(x - cx) / float(rx);
                float dy = float(y - cy) / float(ry);
                if (dx * dx + dy * dy <= 1.0f)
                    grid[Idx(x, y)] = Water;
            }
        }
    }

    void SeedPatch(int cx, int cy, int r, Cell t)
    {
        for (int y = cy - r; y <= cy + r; ++y)
        {
            for (int x = cx - r; x <= cx + r; ++x)
            {
                if (!InBounds(x, y)) continue;
                int dx = x - cx;
                int dy = y - cy;
                if (dx * dx + dy * dy <= r * r && Chance(0.85f))
                    grid[Idx(x, y)] = t;
            }
        }
    }

    void AddRain(int drops)
    {
        for (int i = 0; i < drops; ++i)
        {
            int x = RandInt(0, COLS - 1);
            int y = RandInt(0, ROWS / 2);
            int id = Idx(x, y);
            if (grid[id] != Stone && grid[id] != Tree)
                grid[id] = Water;
        }
    }

    void Reset()
    {
        std::fill(grid.begin(), grid.end(), Soil);

        for (int i = 0; i < (ROWS * COLS) / 65; ++i)
            grid[Idx(RandInt(0, COLS - 1), RandInt(0, ROWS - 1))] = Stone;

        MakePond(COLS / 3, ROWS / 3, 16, 11);
        MakePond(COLS * 3 / 4, ROWS / 2, 11, 8);

        for (int i = 0; i < 12; ++i)
            SeedPatch(RandInt(8, COLS - 9), RandInt(8, ROWS - 9), RandInt(3, 8), Grass);

        for (int i = 0; i < 50; ++i)
        {
            int x = RandInt(0, COLS - 1);
            int y = RandInt(0, ROWS - 1);
            if (grid[Idx(x, y)] == Grass) grid[Idx(x, y)] = Flower;
        }

        for (int i = 0; i < 10; ++i)
        {
            int x = RandInt(0, COLS - 1);
            int y = RandInt(0, ROWS - 1);
            if (grid[Idx(x, y)] == Grass || grid[Idx(x, y)] == Flower)
                grid[Idx(x, y)] = Tree;
        }

        for (int i = 0; i < 30; ++i)
        {
            int x = RandInt(0, COLS - 1);
            int y = RandInt(0, ROWS - 1);
            if (grid[Idx(x, y)] == Soil) grid[Idx(x, y)] = Weed;
        }

        bees.clear();
        bees.reserve(18);
        for (int i = 0; i < 18; ++i)
        {
            Bee b;
            b.pos = {RandFloat(0, (float)WINDOW_SIZE), RandFloat(0, (float)WINDOW_SIZE)};
            b.vel = {RandFloat(-1.0f, 1.0f), RandFloat(-1.0f, 1.0f)};
            b.pollen = false;
            bees.push_back(b);
        }

        season = 0;
        rainTimer = 0.0;
        seasonTimer = 0.0;
        Recount();
    }

    void UpdateBees()
    {
        for (Bee& b : bees)
        {
            int cx = std::clamp((int)(b.pos.x / CELL_SIZE), 0, COLS - 1);
            int cy = std::clamp((int)(b.pos.y / CELL_SIZE), 0, ROWS - 1);

            Vector2 desire = {RandFloat(-0.25f, 0.25f), RandFloat(-0.25f, 0.25f)};
            float bestDist = 999999.0f;
            bool found = false;

            for (int oy = -8; oy <= 8; ++oy)
            {
                for (int ox = -8; ox <= 8; ++ox)
                {
                    int nx = cx + ox;
                    int ny = cy + oy;
                    if (!InBounds(nx, ny)) continue;
                    if (grid[Idx(nx, ny)] != Flower) continue;

                    float d = (float)(ox * ox + oy * oy);
                    if (d < bestDist)
                    {
                        bestDist = d;
                        desire = {(float)ox, (float)oy};
                        found = true;
                    }
                }
            }

            if (found)
            {
                desire = NormalizeSafe(desire);
                b.vel.x += desire.x * 0.18f;
                b.vel.y += desire.y * 0.18f;
            }

            b.vel.x += RandFloat(-0.12f, 0.12f);
            b.vel.y += RandFloat(-0.12f, 0.12f);

            float maxSpeed = (season == 3) ? 0.7f : 1.5f;
            float stepMul  = (season == 3) ? 1.0f : 1.7f;
            b.vel = Limit(b.vel, maxSpeed);

            b.pos.x += b.vel.x * stepMul;
            b.pos.y += b.vel.y * stepMul;

            if (b.pos.x < 0) b.pos.x += WINDOW_SIZE;
            if (b.pos.x >= WINDOW_SIZE) b.pos.x -= WINDOW_SIZE;
            if (b.pos.y < 0) b.pos.y += WINDOW_SIZE;
            if (b.pos.y >= WINDOW_SIZE) b.pos.y -= WINDOW_SIZE;

            int bx = std::clamp((int)(b.pos.x / CELL_SIZE), 0, COLS - 1);
            int by = std::clamp((int)(b.pos.y / CELL_SIZE), 0, ROWS - 1);
            int id = Idx(bx, by);

            if (next[id] == Flower) b.pollen = true;
            if (b.pollen && (next[id] == Grass || next[id] == Soil) && Chance(0.03f))
            {
                next[id] = Flower;
                b.pollen = false;
            }
        }
    }

    void Update(float dt)
    {
        seasonTimer += dt;
        if (seasonTimer > 16.0)
        {
            season = (season + 1) % 4;
            seasonTimer = 0.0;
        }

        rainTimer += dt;
        if (rainTimer > ((season == 1) ? 7.0 : 5.0))
        {
            if (season != 3) AddRain(180);
            rainTimer = 0.0;
        }

        next = grid;

        for (int y = 0; y < ROWS; ++y)
        {
            for (int x = 0; x < COLS; ++x)
            {
                int id = Idx(x, y);
                Cell c = grid[id];

                if (c == Stone) continue;

                int water  = CountAround(x, y, Water);
                int grass  = CountAround(x, y, Grass);
                int flower = CountAround(x, y, Flower);
                int tree   = CountAround(x, y, Tree);
                int weed   = CountAround(x, y, Weed);

                switch (c)
                {
                    case Soil:
                    {
                        float grassGrow =
                            (season == 0) ? 0.10f :
                            (season == 1) ? 0.05f :
                            (season == 2) ? 0.07f : 0.015f;

                        if (weed >= 2 && water == 0 && Chance(0.06f))
                            next[id] = Weed;
                        else if (water > 0 && grass >= 2 && Chance(grassGrow))
                            next[id] = Grass;
                        else if (water > 0 && grass > 0 && Chance(0.015f + 0.01f * flower))
                            next[id] = Flower;
                        break;
                    }

                    case Water:
                    {
                        if (season == 1 && Chance(0.008f))
                            next[id] = Soil;
                        else if (Chance(0.025f))
                        {
                            static const int dx[4] = {1, -1, 0, 0};
                            static const int dy[4] = {0, 0, 1, -1};
                            int start = RandInt(0, 3);

                            for (int k = 0; k < 4; ++k)
                            {
                                int d = (start + k) % 4;
                                int nx = x + dx[d];
                                int ny = y + dy[d];
                                if (!InBounds(nx, ny)) continue;

                                int nid = Idx(nx, ny);
                                if (grid[nid] == Soil || grid[nid] == Grass || grid[nid] == Weed)
                                {
                                    if (Chance(0.18f))
                                        next[nid] = Water;
                                    break;
                                }
                            }
                        }
                        break;
                    }

                    case Grass:
                    {
                        if (weed >= 2 && water == 0 && Chance(0.03f))
                            next[id] = Weed;
                        else if (water > 0 && flower >= 1 && Chance(0.02f))
                            next[id] = Flower;
                        else if (season == 3 && water == 0 && Chance(0.01f))
                            next[id] = Soil;
                        break;
                    }

                    case Flower:
                    {
                        if (season == 1 && water == 0 && Chance(0.02f))
                            next[id] = Grass;
                        else if (flower >= 3 && water > 0 && Chance(0.0045f))
                            next[id] = Tree;
                        break;
                    }

                    case Tree:
                    {
                        if (Chance(0.01f))
                        {
                            int nx = x + RandInt(-2, 2);
                            int ny = y + RandInt(-2, 2);
                            if (InBounds(nx, ny))
                            {
                                int nid = Idx(nx, ny);
                                if (next[nid] == Soil) next[nid] = Grass;
                                else if (next[nid] == Grass && Chance(0.35f)) next[nid] = Flower;
                            }
                        }

                        if (weed >= 4 && Chance(0.01f))
                            next[id] = Grass;
                        break;
                    }

                    case Weed:
                    {
                        if ((tree > 0 || flower >= 2) && Chance(0.05f))
                        {
                            next[id] = Grass;
                        }
                        else if (Chance(0.035f))
                        {
                            int nx = x + RandInt(-1, 1);
                            int ny = y + RandInt(-1, 1);
                            if (InBounds(nx, ny))
                            {
                                int nid = Idx(nx, ny);
                                if (next[nid] == Soil || next[nid] == Grass)
                                    next[nid] = Weed;
                            }
                        }
                        break;
                    }

                    default: break;
                }
            }
        }

        UpdateBees();
        grid.swap(next);
        Recount();
    }

    void Draw() const
    {
        ClearBackground(BackgroundColor());

        for (int y = 0; y < ROWS; ++y)
        {
            for (int x = 0; x < COLS; ++x)
            {
                Color col{};
                switch (grid[Idx(x, y)])
                {
                    case Soil:   col = Color{90, 56, 32, 255}; break;
                    case Water:  col = Color{40, 130, 230, 255}; break;
                    case Grass:  col = Color{60, 180, 75, 255}; break;
                    case Flower: col = Color{255, 170, 220, 255}; break;
                    case Tree:   col = Color{25, 110, 45, 255}; break;
                    case Weed:   col = Color{110, 150, 45, 255}; break;
                    case Stone:  col = Color{110, 110, 110, 255}; break;
                }

                DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, col);
            }
        }

        for (const Bee& b : bees)
        {
            DrawCircleV(b.pos, 2.0f, YELLOW);
            DrawCircleV({b.pos.x + 1.5f, b.pos.y}, 1.0f, ORANGE);
        }

        DrawRectangle(8, 8, 260, 96, Fade(BLACK, 0.60f));
        DrawText("2: Garden Life", 16, 14, 18, RAYWHITE);
        DrawText(TextFormat("Season: %s", SeasonName()), 16, 36, 16, SKYBLUE);
        DrawText(TextFormat("Grass: %d  Flowers: %d", grassCount, flowerCount), 16, 58, 16, GREEN);
        DrawText(TextFormat("Trees: %d  Weeds:   %d", treeCount, weedCount), 16, 78, 16, ORANGE);
    }
};

struct SandboxSim
{
    enum Cell : unsigned char
    {
        Empty,
        Sand,
        Water,
        Plant,
        Fire,
        Stone,
        Glass
    };

    std::vector<Cell> grid;
    std::vector<Cell> next;
    std::vector<unsigned char> fireLife;
    std::vector<unsigned char> nextFireLife;
    double sourceTimer = 0.0;

    int sandCount = 0;
    int waterCount = 0;
    int plantCount = 0;
    int fireCount = 0;

    SandboxSim()
        : grid(ROWS * COLS, Empty),
          next(ROWS * COLS, Empty),
          fireLife(ROWS * COLS, 0),
          nextFireLife(ROWS * COLS, 0)
    {
        Reset();
    }

    int CountAround(int x, int y, Cell t) const
    {
        int count = 0;
        for (int oy = -1; oy <= 1; ++oy)
        {
            for (int ox = -1; ox <= 1; ++ox)
            {
                if (ox == 0 && oy == 0) continue;
                int nx = x + ox;
                int ny = y + oy;
                if (InBounds(nx, ny) && grid[Idx(nx, ny)] == t)
                    ++count;
            }
        }
        return count;
    }

    void Recount()
    {
        sandCount = waterCount = plantCount = fireCount = 0;
        for (Cell c : grid)
        {
            if (c == Sand) ++sandCount;
            else if (c == Water) ++waterCount;
            else if (c == Plant) ++plantCount;
            else if (c == Fire) ++fireCount;
        }
    }

    void Reset()
    {
        std::fill(grid.begin(), grid.end(), Empty);
        std::fill(fireLife.begin(), fireLife.end(), 0);

        for (int y = ROWS - 10; y < ROWS; ++y)
            for (int x = 0; x < COLS; ++x)
                grid[Idx(x, y)] = Stone;

        for (int x = 10; x < 55; ++x)
            for (int y = 30; y < 70; ++y)
                if ((x + y) % 3 != 0)
                    grid[Idx(x, y)] = Sand;

        for (int x = 95; x < 145; ++x)
            for (int y = 35; y < 72; ++y)
                if ((x + y) % 4 != 0)
                    grid[Idx(x, y)] = Water;

        for (int x = 60; x < 92; ++x)
            for (int y = ROWS - 30; y < ROWS - 10; ++y)
                if ((x + y) % 3 == 0)
                    grid[Idx(x, y)] = Plant;

        for (int i = 0; i < 12; ++i)
        {
            int x = RandInt(55, 100);
            int y = RandInt(ROWS - 45, ROWS - 20);
            grid[Idx(x, y)] = Fire;
            fireLife[Idx(x, y)] = (unsigned char)RandInt(12, 24);
        }

        sourceTimer = 0.0;
        Recount();
    }

    bool TryMove(int sx, int sy, int dx, int dy, Cell type)
    {
        if (!InBounds(dx, dy)) return false;

        int s = Idx(sx, sy);
        int d = Idx(dx, dy);

        if (next[d] == Empty && grid[d] == Empty)
        {
            next[d] = type;
            next[s] = Empty;
            nextFireLife[d] = nextFireLife[s];
            nextFireLife[s] = 0;
            return true;
        }

        return false;
    }

    void Update(float dt)
    {
        sourceTimer += dt;
        if (sourceTimer > 0.12f)
        {
            int x = RandInt(3, COLS - 4);
            int pick = RandInt(0, 99);

            if (pick < 45)
                grid[Idx(x, 1)] = Sand;
            else if (pick < 85)
                grid[Idx(x, 1)] = Water;
            else
            {
                grid[Idx(x, 1)] = Fire;
                fireLife[Idx(x, 1)] = (unsigned char)RandInt(10, 22);
            }

            sourceTimer = 0.0;
        }

        next = grid;
        nextFireLife = fireLife;

        for (int y = ROWS - 2; y >= 0; --y)
        {
            bool leftToRight = Chance(0.5f);
            int xStart = leftToRight ? 0 : COLS - 1;
            int xEnd   = leftToRight ? COLS : -1;
            int step   = leftToRight ? 1 : -1;

            for (int x = xStart; x != xEnd; x += step)
            {
                int id = Idx(x, y);
                Cell c = grid[id];

                if (c == Empty || c == Stone || c == Glass) continue;
                if (next[id] != c) continue;

                if (c == Sand)
                {
                    if (TryMove(x, y, x, y + 1, Sand)) continue;

                    if (InBounds(x, y + 1))
                    {
                        int below = Idx(x, y + 1);
                        if (grid[below] == Water && next[below] == Water)
                        {
                            next[below] = Sand;
                            next[id] = Water;
                            continue;
                        }
                    }

                    bool moved = false;
                    int dir = Chance(0.5f) ? -1 : 1;
                    if (TryMove(x, y, x + dir, y + 1, Sand)) moved = true;
                    else if (TryMove(x, y, x - dir, y + 1, Sand)) moved = true;
                    (void)moved;
                }
                else if (c == Water)
                {
                    if (TryMove(x, y, x, y + 1, Water)) continue;

                    int dir = Chance(0.5f) ? -1 : 1;
                    if (TryMove(x, y, x + dir, y + 1, Water)) continue;
                    if (TryMove(x, y, x - dir, y + 1, Water)) continue;
                    if (TryMove(x, y, x + dir, y, Water)) continue;
                    if (TryMove(x, y, x - dir, y, Water)) continue;
                }
                else if (c == Plant)
                {
                    if (CountAround(x, y, Water) > 0 && y > 0 && next[Idx(x, y - 1)] == Empty && Chance(0.03f))
                        next[Idx(x, y - 1)] = Plant;

                    if (CountAround(x, y, Fire) > 0 && Chance(0.22f))
                    {
                        next[id] = Fire;
                        nextFireLife[id] = (unsigned char)RandInt(10, 18);
                    }
                }
                else if (c == Fire)
                {
                    int life = (int)fireLife[id] - 1;
                    bool nearWater = false;

                    for (int oy = -1; oy <= 1; ++oy)
                    {
                        for (int ox = -1; ox <= 1; ++ox)
                        {
                            if (ox == 0 && oy == 0) continue;
                            int nx = x + ox;
                            int ny = y + oy;
                            if (!InBounds(nx, ny)) continue;
                            int nid = Idx(nx, ny);

                            if (grid[nid] == Plant && Chance(0.20f))
                            {
                                next[nid] = Fire;
                                nextFireLife[nid] = (unsigned char)RandInt(10, 20);
                            }
                            else if (grid[nid] == Water)
                            {
                                nearWater = true;
                                if (Chance(0.18f)) next[nid] = Empty;
                            }
                            else if (grid[nid] == Sand && Chance(0.012f))
                            {
                                next[nid] = Glass;
                            }
                        }
                    }

                    if (nearWater) life -= 2;

                    if (life <= 0)
                    {
                        next[id] = Empty;
                        nextFireLife[id] = 0;
                    }
                    else
                    {
                        nextFireLife[id] = (unsigned char)life;
                        if (InBounds(x, y - 1) && next[Idx(x, y - 1)] == Empty && Chance(0.06f))
                        {
                            next[Idx(x, y - 1)] = Fire;
                            nextFireLife[Idx(x, y - 1)] = (unsigned char)std::max(4, life - 3);
                        }
                    }
                }
            }
        }

        grid.swap(next);
        fireLife.swap(nextFireLife);
        Recount();
    }

    void Draw() const
    {
        ClearBackground(Color{8, 8, 12, 255});

        for (int y = 0; y < ROWS; ++y)
        {
            for (int x = 0; x < COLS; ++x)
            {
                Color col = BLACK;
                switch (grid[Idx(x, y)])
                {
                    case Sand:  col = Color{230, 195, 85, 255}; break;
                    case Water: col = Color{60, 140, 255, 255}; break;
                    case Plant: col = Color{70, 185, 70, 255}; break;
                    case Fire:  col = Color{255, 110, 30, 255}; break;
                    case Stone: col = Color{85, 85, 85, 255}; break;
                    case Glass: col = Color{170, 220, 220, 255}; break;
                    default:    col = BLACK; break;
                }

                DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, col);
            }
        }

        DrawRectangle(8, 8, 260, 96, Fade(BLACK, 0.65f));
        DrawText("3: Element Sandbox", 16, 14, 18, RAYWHITE);
        DrawText(TextFormat("Sand: %d  Water: %d", sandCount, waterCount), 16, 38, 16, GOLD);
        DrawText(TextFormat("Plant: %d Fire:  %d", plantCount, fireCount), 16, 60, 16, ORANGE);
        DrawText("Glass forms from hot sand", 16, 82, 14, LIGHTGRAY);
    }
};

enum class Mode
{
    Territory,
    Garden,
    Sandbox
};

int main()
{
    InitWindow(WINDOW_SIZE, WINDOW_SIZE, "Garden / Territory / Sandbox Sim");
    SetTargetFPS(144);

    TerritorySim territory;
    GardenSim garden;
    SandboxSim sandbox;

    Mode mode = Mode::Garden;
    bool paused = false;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        if (IsKeyPressed(KEY_ONE))   mode = Mode::Territory;
        if (IsKeyPressed(KEY_TWO))   mode = Mode::Garden;
        if (IsKeyPressed(KEY_THREE)) mode = Mode::Sandbox;
        if (IsKeyPressed(KEY_SPACE)) paused = !paused;

        if (IsKeyPressed(KEY_R))
        {
            if (mode == Mode::Territory) territory.Reset();
            else if (mode == Mode::Garden) garden.Reset();
            else sandbox.Reset();
        }

        if (!paused)
        {
            if (mode == Mode::Territory) territory.Update(dt);
            else if (mode == Mode::Garden) garden.Update(dt);
            else sandbox.Update(dt);
        }

        BeginDrawing();

        if (mode == Mode::Territory) territory.Draw();
        else if (mode == Mode::Garden) garden.Draw();
        else sandbox.Draw();

        DrawRectangle(8, WINDOW_SIZE - 28, 300, 20, Fade(BLACK, 0.55f));
        DrawText("1/2/3 mode   R reset   SPACE pause", 14, WINDOW_SIZE - 24, 14, RAYWHITE);

        if (paused)
        {
            DrawRectangle(WINDOW_SIZE / 2 - 70, WINDOW_SIZE / 2 - 22, 140, 44, Fade(BLACK, 0.7f));
            DrawText("PAUSED", WINDOW_SIZE / 2 - 38, WINDOW_SIZE / 2 - 10, 20, YELLOW);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}