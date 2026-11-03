// Graphical front end for The Lost Kingdom (SDL2, no extra libraries).
// The game engine is unchanged: typed commands go to Game::handleCommand and the
// engine's text output is shown in the log panel. Scenes are drawn procedurally.
#include "Game.hpp"
#include "font_data.hpp"
#include <SDL.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <sstream>
#include <string>
#include <vector>

using namespace lostkingdom;
using namespace lostkingdom::gui;

namespace {

constexpr int WIN_W = 960, WIN_H = 680;
constexpr int SCENE_H = 380;
constexpr int HUD_Y = SCENE_H, HUD_H = 40;
constexpr int LOG_Y = HUD_Y + HUD_H;
constexpr int INPUT_H = 30;
constexpr int PLAYER_X = 400;
int itemX(size_t i) { return 580 + int(i) * 110; } // floor items sit right of the player

struct Color {
    Uint8 r, g, b, a = 255;
    constexpr Color(int r_, int g_, int b_, int a_ = 255)
        : r(Uint8(r_ < 0 ? 0 : r_ > 255 ? 255 : r_)), g(Uint8(g_ < 0 ? 0 : g_ > 255 ? 255 : g_)),
          b(Uint8(b_ < 0 ? 0 : b_ > 255 ? 255 : b_)), a(Uint8(a_)) {}
};

SDL_Renderer* gr = nullptr;
Uint32 gTicks = 0;

void setColor(Color c) { SDL_SetRenderDrawColor(gr, c.r, c.g, c.b, c.a); }
void rect(int x, int y, int w, int h, Color c) {
    setColor(c);
    SDL_Rect r{x, y, w, h};
    SDL_RenderFillRect(gr, &r);
}
void frame(int x, int y, int w, int h, Color c) {
    setColor(c);
    SDL_Rect r{x, y, w, h};
    SDL_RenderDrawRect(gr, &r);
}
void vgradient(int x, int y, int w, int h, Color top, Color bottom) {
    for (int i = 0; i < h; ++i) {
        float t = h > 1 ? float(i) / (h - 1) : 0.f;
        setColor({Uint8(top.r + (bottom.r - top.r) * t), Uint8(top.g + (bottom.g - top.g) * t),
                  Uint8(top.b + (bottom.b - top.b) * t)});
        SDL_RenderDrawLine(gr, x, y + i, x + w - 1, y + i);
    }
}
void disc(int cx, int cy, int radius, Color c) {
    setColor(c);
    for (int dy = -radius; dy <= radius; ++dy) {
        int dx = int(std::sqrt(float(radius * radius - dy * dy)));
        SDL_RenderDrawLine(gr, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}
void glow(int cx, int cy, int radius, Color c) {
    SDL_SetRenderDrawBlendMode(gr, SDL_BLENDMODE_ADD);
    for (int r = radius; r > 0; r -= 3) disc(cx, cy, r, {c.r, c.g, c.b, Uint8(std::max(2, 40 * (radius - r) / radius + 3))});
    SDL_SetRenderDrawBlendMode(gr, SDL_BLENDMODE_BLEND);
}
void triangle(int x, int y, int w, int h, Color c) { // apex at top centre
    setColor(c);
    for (int i = 0; i < h; ++i) {
        int half = w * i / (2 * h);
        SDL_RenderDrawLine(gr, x + w / 2 - half, y + i, x + w / 2 + half, y + i);
    }
}

// ---- text -------------------------------------------------------------
SDL_Texture* gFont = nullptr;

void buildFont() {
    constexpr int count = FONT_LAST - FONT_FIRST + 1;
    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormat(0, count * FONT_W, FONT_H, 32, SDL_PIXELFORMAT_RGBA32);
    auto* px = static_cast<Uint32*>(surf->pixels);
    for (int g = 0; g < count; ++g)
        for (int y = 0; y < FONT_H; ++y)
            for (int x = 0; x < FONT_W; ++x) {
                bool on = FONT_DATA[g][y] & (1 << (7 - x));
                px[y * (surf->pitch / 4) + g * FONT_W + x] = on ? 0xFFFFFFFFu : 0u;
            }
    gFont = SDL_CreateTextureFromSurface(gr, surf);
    SDL_FreeSurface(surf);
}
int textWidth(const std::string& s, int scale = 1) { return int(s.size()) * FONT_W * scale; }
void text(int x, int y, const std::string& s, Color c, int scale = 1) {
    SDL_SetTextureColorMod(gFont, c.r, c.g, c.b);
    SDL_SetTextureAlphaMod(gFont, c.a);
    for (char ch : s) {
        if (ch >= FONT_FIRST && ch <= FONT_LAST) {
            SDL_Rect src{(ch - FONT_FIRST) * FONT_W, 0, FONT_W, FONT_H};
            SDL_Rect dst{x, y, FONT_W * scale, FONT_H * scale};
            SDL_RenderCopy(gr, gFont, &src, &dst);
        }
        x += FONT_W * scale;
    }
}
void textCentered(int cx, int y, const std::string& s, Color c, int scale = 1) {
    text(cx - textWidth(s, scale) / 2, y, s, c, scale);
}
std::vector<std::string> wrap(const std::string& s, int cols) {
    std::vector<std::string> out;
    std::istringstream lines(s);
    std::string line;
    while (std::getline(lines, line)) {
        std::string cur, word;
        std::istringstream words(line);
        if (line.empty()) out.push_back("");
        while (words >> word) {
            if (!cur.empty() && int(cur.size() + 1 + word.size()) > cols) { out.push_back(cur); cur.clear(); }
            cur += (cur.empty() ? "" : " ") + word;
        }
        if (!cur.empty()) out.push_back(cur);
    }
    return out;
}

// ---- sprites ----------------------------------------------------------
void drawPlayer(int x, int y, int bob) { // x = centre, y = feet
    y += bob;
    rect(x - 9, y - 4, 7, 4, {40, 30, 30});          // boots
    rect(x + 2, y - 4, 7, 4, {40, 30, 30});
    rect(x - 9, y - 22, 18, 18, {50, 90, 170});      // tunic
    rect(x - 9, y - 12, 18, 3, {210, 170, 60});      // belt
    rect(x - 13, y - 21, 5, 13, {50, 90, 170});      // arms
    rect(x + 8, y - 21, 5, 13, {50, 90, 170});
    rect(x - 7, y - 36, 14, 14, {235, 195, 160});    // head
    rect(x - 8, y - 38, 16, 5, {120, 70, 30});       // hair
    rect(x - 4, y - 31, 2, 2, {20, 20, 30});         // eyes
    rect(x + 2, y - 31, 2, 2, {20, 20, 30});
    rect(x - 12, y - 24, 24, 3, {170, 40, 40});      // scarf
}
void drawGuide(int x, int y) {
    rect(x - 10, y - 30, 20, 30, {120, 60, 150});
    triangle(x - 12, y - 52, 24, 24, {160, 90, 190}); // hood
    rect(x - 5, y - 40, 10, 9, {235, 195, 160});
    rect(x - 3, y - 37, 2, 2, {20, 20, 30}); rect(x + 1, y - 37, 2, 2, {20, 20, 30});
    rect(x + 14, y - 46, 3, 46, {110, 80, 40});       // staff
    float pulse = 0.5f + 0.5f * std::sin(gTicks / 300.f);
    glow(x + 15, y - 50, int(18 + 6 * pulse), {120, 200, 255});
    disc(x + 15, y - 50, 4, {210, 240, 255});
}
void drawGuardian(int x, int y, bool awake) {
    Color stone = awake ? Color{120, 124, 132} : Color{70, 72, 78};
    rect(x - 26, y - 70, 52, 70, stone);                // body
    rect(x - 38, y - 66, 14, 46, stone); rect(x + 24, y - 66, 14, 46, stone); // arms
    rect(x - 18, y - 100, 36, 32, {stone.r + 12, stone.g + 12, stone.b + 14});  // head
    rect(x - 12, y - 90, 8, 6, awake ? Color{255, 90, 40} : Color{30, 30, 34});
    rect(x + 4, y - 90, 8, 6, awake ? Color{255, 90, 40} : Color{30, 30, 34});
    for (int i = 0; i < 4; ++i) rect(x - 20 + i * 11, y - 50, 3, 24, {stone.r - 25, stone.g - 25, stone.b - 25}); // cracks/rune lines
    if (awake) glow(x, y - 87, 36, {255, 90, 40});
    else { rect(x - 30, y - 8, 60, 3, {100, 200, 255}); }
}

// ---- item icons -------------------------------------------------------
void drawItemIcon(const std::string& name, int cx, int cy, bool glowing = true) {
    float pulse = 0.5f + 0.5f * std::sin(gTicks / 250.f);
    if (name == "ANCIENT SEAL") {
        if (glowing) glow(cx, cy, int(26 + 5 * pulse), {255, 200, 80});
        disc(cx, cy, 11, {200, 150, 40}); disc(cx, cy, 8, {240, 200, 90});
        rect(cx - 1, cy - 6, 2, 12, {150, 100, 20}); rect(cx - 6, cy - 1, 12, 2, {150, 100, 20});
    } else if (name == "LUMINOUS KEY") {
        if (glowing) glow(cx, cy, int(28 + 6 * pulse), {120, 220, 255});
        disc(cx - 8, cy, 7, {170, 235, 255}); disc(cx - 8, cy, 3, {20, 40, 60});
        rect(cx - 2, cy - 2, 18, 4, {170, 235, 255});
        rect(cx + 10, cy, 3, 7, {170, 235, 255}); rect(cx + 15, cy, 3, 5, {170, 235, 255});
    } else if (name == "ROYAL COIN") {
        if (glowing) glow(cx, cy, int(20 + 4 * pulse), {255, 220, 100});
        disc(cx, cy, 10, {190, 140, 30}); disc(cx, cy, 8, {250, 215, 80});
        rect(cx - 2, cy - 5, 4, 10, {190, 140, 30});
    } else {
        disc(cx, cy, 8, {200, 200, 200});
    }
}

// ---- room scenes ------------------------------------------------------
constexpr int FLOOR_Y = 290;

void sceneCourtyard(bool) {
    vgradient(0, 0, WIN_W, FLOOR_Y, {18, 24, 60}, {90, 70, 110});
    for (int i = 0; i < 40; ++i) { // stars
        int sx = (i * 197) % WIN_W, sy = (i * 83) % 150;
        if ((gTicks / 400 + i) % 7) rect(sx, sy, 2, 2, {230, 230, 255});
    }
    disc(780, 70, 30, {240, 236, 210}); disc(768, 62, 28, {18, 24, 60}); // crescent moon
    rect(0, 190, WIN_W, 100, {60, 56, 80});                              // far wall
    for (int x = 0; x < WIN_W; x += 80) rect(x, 175, 40, 20, {60, 56, 80});
    rect(120, 60, 90, 130, {70, 66, 90});                                // broken tower
    triangle(110, 30, 110, 40, {50, 46, 70});
    rect(420, 110, 120, 80, {120, 30, 40}); rect(420, 110, 120, 6, {200, 160, 50}); // royal crest banner
    disc(480, 148, 18, {210, 170, 60}); disc(480, 148, 12, {120, 30, 40});
    vgradient(0, FLOOR_Y, WIN_W, SCENE_H - FLOOR_Y, {88, 82, 100}, {50, 46, 64});
    for (int x = 0; x < WIN_W; x += 60) SDL_RenderDrawLine(gr, x, FLOOR_Y, x - 40 + (x / 4), SCENE_H);
    // cracked fountain
    rect(660, 300, 150, 18, {130, 130, 145}); rect(675, 282, 120, 20, {110, 110, 125});
    rect(720, 230, 30, 55, {150, 150, 165}); disc(735, 228, 14, {150, 150, 165});
    for (int i = 0; i < 5; ++i) {
        int dy = int(((gTicks / 60) + i * 9) % 40);
        rect(731 + (i - 2) * 4, 235 + dy, 2, 3, {120, 190, 255});
    }
    rect(740, 255, 2, 40, {20, 20, 30}); // crack
}
void sceneLibrary(bool) {
    vgradient(0, 0, WIN_W, FLOOR_Y, {46, 30, 22}, {80, 52, 34});
    for (int s = 0; s < 4; ++s) { // bookshelves
        int bx = 30 + s * 230;
        rect(bx, 40, 190, 250, {60, 36, 20});
        for (int row = 0; row < 4; ++row) {
            rect(bx + 6, 50 + row * 60 + 50, 178, 5, {90, 60, 30});
            int x = bx + 8;
            for (int k = 0; k < 14; ++k) {
                int bw = 7 + (k * 5 + row * 3 + s) % 7;
                Color col{Uint8(80 + (k * 37 + row * 11) % 120), Uint8(40 + (k * 53 + s * 17) % 80), Uint8(40 + (k * 71) % 90)};
                rect(x, 56 + row * 60 + (k % 3), bw, 44 - (k % 3), col);
                x += bw + 2;
                if (x > bx + 178) break;
            }
        }
    }
    rect(0, FLOOR_Y, WIN_W, SCENE_H - FLOOR_Y, {70, 44, 28});
    for (int x = 0; x < WIN_W; x += 48) rect(x, FLOOR_Y, 2, SCENE_H - FLOOR_Y, {50, 30, 18});
    for (int i = 0; i < 18; ++i) { // dust motes
        int dx = int((i * 211 + gTicks / 40) % WIN_W), dy = int(60 + (i * 97 + gTicks / 55) % 220);
        rect(dx, dy, 2, 2, {255, 230, 170, 120});
    }
}
void sceneGreenhouse(bool) {
    vgradient(0, 0, WIN_W, FLOOR_Y, {120, 190, 170}, {190, 230, 200});
    setColor({50, 90, 70}); // glass frame
    for (int x = 0; x <= WIN_W; x += 120) SDL_RenderDrawLine(gr, x, 0, WIN_W / 2 + (x - WIN_W / 2) / 3, 130);
    for (int y = 0; y < FLOOR_Y; y += 60) SDL_RenderDrawLine(gr, 0, y, WIN_W, y);
    for (int x = 20; x < WIN_W; x += 70) { // hanging vines
        int len = 60 + (x * 7) % 120;
        rect(x, 0, 4, len, {40, 120, 50});
        for (int y = 12; y < len; y += 22) disc(x + ((y / 22) % 2 ? 8 : -4), y, 7, {60, 160, 70});
    }
    rect(0, FLOOR_Y, WIN_W, SCENE_H - FLOOR_Y, {110, 80, 50});
    for (int x = 40; x < WIN_W; x += 150) { // plant pots
        rect(x, FLOOR_Y - 30, 40, 30, {170, 90, 60});
        for (int l = 0; l < 5; ++l) triangle(x - 10 + l * 11, FLOOR_Y - 85 + (l % 2) * 15, 14, 60, {50, 150 + l * 10, 70});
    }
}
void sceneHall(bool) {
    vgradient(0, 0, WIN_W, FLOOR_Y, {20, 40, 70}, {40, 80, 110});
    for (int p = 0; p < 5; ++p) { // pillars
        int px = 40 + p * 220;
        rect(px, 0, 50, FLOOR_Y, {70, 90, 110}); rect(px - 8, 0, 66, 18, {90, 110, 130}); rect(px - 8, FLOOR_Y - 14, 66, 14, {90, 110, 130});
    }
    for (int m = 0; m < 3; ++m) { // three seal murals
        int mx = 190 + m * 220;
        rect(mx, 80, 70, 100, {40, 40, 60}); frame(mx, 80, 70, 100, {200, 170, 80});
        float pulse = 0.5f + 0.5f * std::sin(gTicks / 400.f + m);
        glow(mx + 35, 130, int(22 + 5 * pulse), {200, 170, 80});
        disc(mx + 35, 130, 12, {210, 180, 90});
    }
    vgradient(0, FLOOR_Y, WIN_W, SCENE_H - FLOOR_Y, {30, 90, 130}, {15, 50, 90}); // water
    for (int i = 0; i < 12; ++i) {
        int wx = (i * 83 + gTicks / 25) % WIN_W, wy = FLOOR_Y + 8 + (i * 29) % (SCENE_H - FLOOR_Y - 12);
        rect(wx, wy, 30, 2, {140, 200, 240, 140});
    }
}
void sceneTower(bool guardianDefeated) {
    vgradient(0, 0, WIN_W, FLOOR_Y, {30, 20, 50}, {170, 90, 80});
    disc(200, 230, 55, {250, 200, 120});
    for (int i = 0; i < 6; ++i) rect(0, 180 + i * 18, WIN_W, 3, {170, 90, 80, 90});
    rect(0, 120, 120, 170, {50, 46, 60}); rect(WIN_W - 120, 90, 120, 200, {50, 46, 60});
    for (int x = 0; x < 120; x += 40) { rect(x, 100, 24, 20, {50, 46, 60}); rect(WIN_W - 120 + x, 70, 24, 20, {50, 46, 60}); }
    // northern arch
    rect(380, 70, 200, 220, {70, 66, 84});
    disc(480, 130, 70, {70, 66, 84});
    disc(480, 140, 50, guardianDefeated ? Color{250, 230, 160} : Color{12, 10, 20});
    rect(430, 140, 100, 150, guardianDefeated ? Color{250, 230, 160} : Color{12, 10, 20});
    if (guardianDefeated) glow(480, 200, 110, {255, 230, 150});
    else { // sealed
        rect(430, 150, 100, 140, {30, 30, 44});
        float pulse = 0.5f + 0.5f * std::sin(gTicks / 350.f);
        glow(480, 210, int(34 + 6 * pulse), {100, 190, 255});
        frame(448, 168, 64, 64, {100, 190, 255});
        disc(480, 200, 14, {100, 190, 255}); disc(480, 200, 8, {12, 20, 40});
    }
    rect(0, FLOOR_Y, WIN_W, SCENE_H - FLOOR_Y, {56, 50, 66});
    for (int x = 0; x < WIN_W; x += 70) rect(x, FLOOR_Y, 2, SCENE_H - FLOOR_Y, {38, 34, 48});
}
void sceneVault(bool solved) {
    vgradient(0, 0, WIN_W, FLOOR_Y, {10, 8, 24}, {30, 18, 50});
    for (int i = 0; i < 9; ++i) { // golden runes
        int rx = 60 + i * 105;
        rect(rx, 50 + (i % 3) * 25, 6, 30, {120, 90, 30}); rect(rx - 8, 60 + (i % 3) * 25, 22, 5, {120, 90, 30});
    }
    rect(0, FLOOR_Y, WIN_W, SCENE_H - FLOOR_Y, {34, 26, 50});
    rect(390, 245, 180, 46, {90, 80, 110}); rect(410, 215, 140, 32, {110, 100, 130}); // altar
    float pulse = 0.5f + 0.5f * std::sin(gTicks / 300.f);
    if (solved) {
        glow(480, 190, int(130 + 20 * pulse), {255, 210, 100});
        drawItemIcon("ANCIENT SEAL", 480, 196, false);
        for (int i = 0; i < 16; ++i) { // rising sparkles
            int sx = 400 + (i * 53) % 160, sy = 200 - int((gTicks / 12 + i * 41) % 180);
            rect(sx, sy, 3, 3, {255, 230, 140});
        }
    } else {
        glow(480, 200, int(40 + 10 * pulse), {160, 120, 255});
        rect(468, 200, 24, 10, {50, 40, 70}); // empty socket
    }
}

void drawScene(int roomId, bool guardianDefeated, bool puzzleSolved) {
    switch (roomId) {
        case 1: sceneCourtyard(false); break;
        case 2: sceneLibrary(false); break;
        case 3: sceneGreenhouse(false); break;
        case 4: sceneHall(false); break;
        case 5: sceneTower(guardianDefeated); break;
        case 6: sceneVault(puzzleSolved); break;
        default: rect(0, 0, WIN_W, SCENE_H, {0, 0, 0});
    }
}

// ---- overlays ---------------------------------------------------------
struct Rooms { int id, x, y; const char* label; };
const Rooms MAP_ROOMS[] = {{1, 0, 1, "Court"}, {2, 0, 0, "Lib"}, {3, 1, 1, "Green"},
                           {4, 1, 0, "Hall"}, {5, 2, 0, "Tower"}, {6, 2, -1, "Vault"}};

void drawMiniMap(int currentRoom) {
    const int ox = WIN_W - 190, oy = 36, cw = 52, ch = 38, gap = 6;
    SDL_SetRenderDrawBlendMode(gr, SDL_BLENDMODE_BLEND);
    rect(ox - 10, oy - 28, 3 * (cw + gap) + 14, 3 * (ch + gap) + 32, {10, 10, 20, 180});
    text(ox, oy - 22, "MAP", {200, 200, 220});
    auto center = [&](const Rooms& r) { return std::pair<int, int>{ox + r.x * (cw + gap) + cw / 2, oy + (r.y + 1) * (ch + gap) + ch / 2}; };
    setColor({90, 90, 120});
    const int links[][2] = {{1, 2}, {1, 3}, {2, 4}, {3, 4}, {4, 5}, {5, 6}};
    for (auto& l : links) {
        auto a = center(MAP_ROOMS[l[0] - 1]), b = center(MAP_ROOMS[l[1] - 1]);
        SDL_RenderDrawLine(gr, a.first, a.second, b.first, b.second);
    }
    for (const auto& r : MAP_ROOMS) {
        auto c = center(r);
        bool here = r.id == currentRoom;
        rect(c.first - cw / 2, c.second - ch / 2, cw, ch, here ? Color{230, 180, 60} : Color{40, 44, 70});
        frame(c.first - cw / 2, c.second - ch / 2, cw, ch, {160, 160, 200});
        textCentered(c.first, c.second - 8, r.label, here ? Color{20, 20, 20} : Color{200, 200, 230});
    }
}

void drawExits(const Room& room) {
    const int cx = WIN_W / 2;
    auto marker = [&](const char* dir, const char* label, int x, int y, int dx, int dy) {
        if (!room.hasExit(dir)) return;
        float bob = 3 * std::sin(gTicks / 250.f);
        int ax = x + int(dx * bob), ay = y + int(dy * bob);
        int w = textWidth(label) + 16;
        rect(ax - w / 2, ay - 12, w, 24, {10, 10, 20, 170});
        frame(ax - w / 2, ay - 12, w, 24, {255, 230, 140});
        textCentered(ax, ay - 8, label, {255, 230, 140});
    };
    marker("NORTH", "^ NORTH", cx, 56, 0, -1);
    marker("SOUTH", "v SOUTH", cx, SCENE_H - 18, 0, 1);
    marker("WEST", "< WEST", 60, 250, -1, 0);
    marker("EAST", "EAST >", WIN_W - 60, 250, 1, 0);
}

void drawHud(const Game& game) {
    rect(0, HUD_Y, WIN_W, HUD_H, {18, 18, 32});
    frame(0, HUD_Y, WIN_W, HUD_H, {70, 70, 110});
    const Player& p = game.player();
    text(10, HUD_Y + 4, "HP", {255, 130, 130});
    rect(36, HUD_Y + 6, 150, 12, {60, 20, 20});
    int hw = 150 * p.health() / 100;
    rect(36, HUD_Y + 6, hw, 12, p.health() > 50 ? Color{80, 200, 90} : p.health() > 25 ? Color{230, 190, 60} : Color{220, 60, 60});
    frame(36, HUD_Y + 6, 150, 12, {200, 200, 220});
    text(194, HUD_Y + 4, std::to_string(p.health()), {255, 255, 255});
    text(10, HUD_Y + 22, "SCORE " + std::to_string(p.score()), {255, 220, 120});
    text(240, HUD_Y + 4, game.currentRoom().name(), {200, 220, 255});
    // inventory slots
    const int sx = 520, slot = 30;
    text(sx - 46, HUD_Y + 12, "ITEMS", {180, 180, 210});
    for (int i = 0; i < 6; ++i) {
        rect(sx + i * (slot + 4), HUD_Y + 5, slot, slot, {30, 30, 52});
        frame(sx + i * (slot + 4), HUD_Y + 5, slot, slot, {110, 110, 160});
        if (i < int(p.inventory().size()))
            drawItemIcon(p.inventory()[i]->name(), sx + i * (slot + 4) + slot / 2, HUD_Y + 5 + slot / 2, false);
    }
}

struct LogLine { std::string text; Color color; };

// Saves the current frame as a BMP (used by --demo to produce screenshots).
void saveShot(const std::string& path) {
    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormat(0, WIN_W, WIN_H, 32, SDL_PIXELFORMAT_ARGB8888);
    if (!surf) return;
    SDL_RenderReadPixels(gr, nullptr, SDL_PIXELFORMAT_ARGB8888, surf->pixels, surf->pitch);
    SDL_SaveBMP(surf, path.c_str());
    SDL_FreeSurface(surf);
}

} // namespace

int main(int argc, char** argv) {
    // --demo <dir>: play a scripted walkthrough and save a screenshot after each step, then exit.
    std::string demoDir;
    for (int i = 1; i + 1 < argc; ++i)
        if (std::string(argv[i]) == "--demo") demoDir = argv[i + 1];
    const std::vector<std::string> demoSteps = {"TALK", "MOVE NORTH", "PICK ANCIENT SEAL", "MOVE SOUTH", "MOVE EAST",
        "PICK LUMINOUS KEY", "MOVE SOUTH", "MOVE EAST", "ATTACK", "USE LUMINOUS KEY", "MOVE NORTH", "USE ANCIENT SEAL"};
    size_t demoIndex = 0;
    std::vector<bool> shotTaken(demoSteps.size() + 1, false);
    Uint32 demoStart = 0;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }
    SDL_Window* window = SDL_CreateWindow("The Lost Kingdom", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WIN_W, WIN_H, SDL_WINDOW_SHOWN);
    if (!window) { SDL_Log("Window failed: %s", SDL_GetError()); return 1; }
    gr = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!gr) gr = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!gr) { SDL_Log("Renderer failed: %s", SDL_GetError()); return 1; }
    SDL_SetRenderDrawBlendMode(gr, SDL_BLENDMODE_BLEND);
    buildFont();

    std::ostringstream engineOut;
    Game game(engineOut);
    std::vector<LogLine> log;
    std::string input;
    int scroll = 0;

    auto pump = [&]() { // move new engine text into the log
        std::string s = engineOut.str();
        engineOut.str(""); engineOut.clear();
        for (auto& line : wrap(s, (WIN_W - 24) / FONT_W)) {
            Color c{210, 214, 230};
            if (line.rfind("VICTORY", 0) == 0) c = {255, 220, 90};
            else if (line.find("GAME OVER") != std::string::npos || line.rfind("Error", 0) == 0 || line.find("cannot") != std::string::npos) c = {255, 130, 120};
            else if (line.rfind("Collected", 0) == 0 || line.find("opens") != std::string::npos) c = {140, 230, 150};
            log.push_back({line, c});
        }
        scroll = 0;
        if (log.size() > 500) log.erase(log.begin(), log.begin() + (log.size() - 500));
    };
    auto submit = [&](const std::string& cmd) {
        log.push_back({"> " + cmd, {120, 200, 255}});
        game.handleCommand(cmd);
        pump();
    };

    game.printIntro();
    pump();
    log.push_back({"Arrow keys move. Type commands (HELP) and press Enter. Click an item to pick it up.", {160, 160, 190}});

    SDL_StartTextInput();
    bool quit = false;
    int playerX = PLAYER_X;
    int lastRoom = game.player().roomId();
    Uint32 fade = 0;
    Uint32 endAt = 0;

    while (!quit) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) quit = true;
            else if (ev.type == SDL_TEXTINPUT && input.size() < 60) input += ev.text.text;
            else if (ev.type == SDL_KEYDOWN) {
                switch (ev.key.keysym.sym) {
                    case SDLK_BACKSPACE: if (!input.empty()) input.pop_back(); break;
                    case SDLK_RETURN: case SDLK_KP_ENTER: if (!input.empty()) { submit(input); input.clear(); } break;
                    case SDLK_ESCAPE: quit = true; break;
                    case SDLK_UP: if (game.running()) submit("MOVE NORTH"); break;
                    case SDLK_DOWN: if (game.running()) submit("MOVE SOUTH"); break;
                    case SDLK_LEFT: if (game.running()) submit("MOVE WEST"); break;
                    case SDLK_RIGHT: if (game.running()) submit("MOVE EAST"); break;
                    case SDLK_PAGEUP: scroll = std::min(scroll + 4, std::max(0, int(log.size()) - 1)); break;
                    case SDLK_PAGEDOWN: scroll = std::max(scroll - 4, 0); break;
                }
            } else if (ev.type == SDL_MOUSEWHEEL) {
                scroll = std::clamp(scroll + ev.wheel.y * 2, 0, std::max(0, int(log.size()) - 1));
            } else if (ev.type == SDL_MOUSEBUTTONDOWN && ev.button.button == SDL_BUTTON_LEFT && game.running()) {
                // click on a floor item to pick it up
                const auto& items = game.currentRoom().items();
                for (size_t i = 0; i < items.size(); ++i) {
                    int ix = itemX(i), iy = 305;
                    if (std::abs(ev.button.x - ix) < 26 && std::abs(ev.button.y - iy) < 26) { submit("PICK " + items[i]->name()); break; }
                }
            }
        }
        gTicks = SDL_GetTicks();
        if (!demoDir.empty()) {
            if (!demoStart) demoStart = gTicks;
            Uint32 t = gTicks - demoStart;
            size_t step = t / 1200;
            if (step < demoSteps.size() && step == demoIndex) { submit(demoSteps[step]); ++demoIndex; }
            else if (step >= demoSteps.size() + 1) quit = true;
        }

        int room = game.player().roomId();
        if (room != lastRoom) { lastRoom = room; fade = gTicks; playerX = PLAYER_X; }
        if (!game.running() && !endAt) endAt = gTicks;

        setColor({0, 0, 0});
        SDL_RenderClear(gr);
        drawScene(room, game.guardianDefeated(), game.puzzleSolved());

        const Room& cur = game.currentRoom();
        if (room == 1) drawGuide(250, 318);
        if (room == 5) drawGuardian(190, 300, !game.guardianDefeated());
        // floor items
        const auto& items = cur.items();
        for (size_t i = 0; i < items.size(); ++i) {
            int ix = itemX(i), iy = 305;
            rect(ix - 14, iy + 12, 28, 4, {0, 0, 0, 90});
            drawItemIcon(items[i]->name(), ix, iy + int(3 * std::sin(gTicks / 300.f + i)));
            textCentered(ix, iy + 24, items[i]->name(), {255, 245, 200});
        }
        drawPlayer(playerX, 345, int(2 * std::sin(gTicks / 400.f)));
        drawExits(cur);
        if (room == 1) textCentered(250, 232, "Mira", {220, 190, 255});
        if (room == 5) textCentered(190, 140, "Stone Guardian", {255, 200, 160});
        drawMiniMap(room);
        textCentered(WIN_W / 2, 6, cur.name(), {255, 255, 255}, 2);

        // fade-in when entering a room
        if (gTicks - fade < 400) rect(0, 0, WIN_W, SCENE_H, {0, 0, 0, Uint8(255 - 255 * (gTicks - fade) / 400)});

        drawHud(game);

        // log
        rect(0, LOG_Y, WIN_W, WIN_H - LOG_Y, {12, 12, 22});
        const int lines = (WIN_H - LOG_Y - INPUT_H - 8) / FONT_H;
        int end = std::max(0, int(log.size()) - scroll), start = std::max(0, end - lines);
        for (int i = start; i < end; ++i) text(12, LOG_Y + 4 + (i - start) * FONT_H, log[i].text, log[i].color);
        // input
        int iy = WIN_H - INPUT_H;
        rect(0, iy, WIN_W, INPUT_H, {24, 24, 44});
        frame(0, iy, WIN_W, INPUT_H, {70, 70, 110});
        text(12, iy + 7, "> " + input + ((gTicks / 500) % 2 ? "_" : " "), {255, 255, 255});

        if (!game.running()) {
            rect(0, 0, WIN_W, SCENE_H, {0, 0, 0, Uint8(std::min<Uint32>(170, (gTicks - endAt) / 5))});
            if (gTicks - endAt > 400) {
                const bool won = game.won();
                textCentered(WIN_W / 2, 140, won ? "VICTORY" : (game.player().health() <= 0 ? "GAME OVER" : "FAREWELL"),
                             won ? Color{255, 220, 90} : Color{255, 120, 110}, 6);
                textCentered(WIN_W / 2, 250, "Score: " + std::to_string(game.player().score()), {255, 255, 255}, 2);
                textCentered(WIN_W / 2, 290, "Press Esc to close", {200, 200, 220});
            }
        }
        if (!demoDir.empty() && demoIndex > 0 && (gTicks - demoStart) % 1200 > 900 && !shotTaken[demoIndex - 1]) {
            shotTaken[demoIndex - 1] = true;
            char name[32];
            std::snprintf(name, sizeof(name), "/step_%02d.bmp", int(demoIndex - 1));
            saveShot(demoDir + name);
        }
        SDL_RenderPresent(gr);
    }
    SDL_DestroyTexture(gFont);
    SDL_DestroyRenderer(gr);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
