#include "VisualGame.hpp"
#include "c_utils.h"

#include "raylib.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

namespace {
constexpr int SCREEN_W = 1280;
constexpr int SCREEN_H = 800;
constexpr int COLS = 12;
constexpr int ROWS = 8;
constexpr float TILE = 64.0f;
constexpr float MAP_X = 42.0f;
constexpr float MAP_Y = 162.0f;
constexpr float MAP_W = COLS * TILE;
constexpr float MAP_H = ROWS * TILE;
constexpr float PLAYER_SPEED = 240.0f;

const Color UI_INK       = { 12, 17, 29, 255 };
const Color UI_PANEL     = { 23, 31, 49, 255 };
const Color UI_PANEL2    = { 30, 41, 63, 255 };
const Color UI_GOLD      = { 245, 190, 86, 255 };
const Color UI_PALE_GOLD = { 255, 224, 155, 255 };
const Color UI_TEXT      = { 235, 239, 248, 255 };
const Color UI_MUTED     = { 151, 166, 190, 255 };
const Color UI_GREEN     = { 88, 170, 117, 255 };
const Color UI_PURPLE    = { 143, 111, 210, 255 };
const Color UI_RED       = { 217, 101, 105, 255 };

struct Spot {
    float x;
    float y;
};

const std::array<const char*, 7> ROOM_NAMES = {
    "", "Arrival Courtyard", "Old Library", "Greenhouse",
    "Sunken Hall", "Watchtower", "Hidden Vault"
};

Spot tileCenter(int col, int row) {
    return { MAP_X + (col + 0.5f) * TILE, MAP_Y + (row + 0.5f) * TILE };
}

float distanceBetween(Spot a, Spot b) {
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    return std::sqrt(dx*dx + dy*dy);
}

Color floorColor(int room, int col, int row) {
    const bool alternate = ((col + row) % 2) == 0;
    switch (room) {
        case 1: return alternate ? Color{45, 86, 67, 255} : Color{42, 79, 62, 255};
        case 2: return alternate ? Color{91, 65, 48, 255} : Color{83, 58, 44, 255};
        case 3: return alternate ? Color{59, 104, 72, 255} : Color{52, 95, 67, 255};
        case 4: return alternate ? Color{53, 72, 88, 255} : Color{47, 65, 82, 255};
        case 5: return alternate ? Color{84, 82, 95, 255} : Color{74, 73, 88, 255};
        default:return alternate ? Color{69, 58, 103, 255}: Color{61, 51, 94, 255};
    }
}

class LostKingdomWindow {
public:
    int run() {
        SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
        InitWindow(SCREEN_W, SCREEN_H, "THE LOST KINGDOM  |  A 2D ADVENTURE");
        if (!IsWindowReady()) return 1;
        SetTargetFPS(60);

        while (!WindowShouldClose() && !quit_) {
            const float dt = std::min(GetFrameTime(), 0.04f);
            if (startScreen_) {
                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) startScreen_ = false;
            } else if (!victory_) {
                update(dt);
            }

            BeginDrawing();
            draw();
            EndDrawing();
        }

        CloseWindow();
        return 0;
    }

private:
    int room_ = 1;
    Spot player_ = tileCenter(5, 6);
    float animation_ = 0.0f;
    float interactionPulse_ = 0.0f;
    int health_ = 100;
    int score_ = 0;

    bool startScreen_ = true;
    bool quit_ = false;
    bool victory_ = false;
    bool guideSpoken_ = false;
    bool hasSeal_ = false;
    bool hasKey_ = false;
    bool hasCoin_ = false;
    bool guardianDefeated_ = false;
    bool showMap_ = false;
    bool showInventory_ = false;
    bool commandMode_ = false;
    bool saveExists_ = false;

    std::string event_ = "Find Mira the Guide and begin your quest.";
    std::string commandLine_;
    std::vector<std::string> log_ = {
        "A forgotten kingdom waits beyond the courtyard.",
        "Find the Ancient Seal and the Luminous Key."
    };

    bool hasItem(const std::string& item) const {
        if (item == "ANCIENT SEAL") return hasSeal_;
        if (item == "LUMINOUS KEY") return hasKey_;
        if (item == "ROYAL COIN") return hasCoin_;
        return false;
    }

    void addLog(const std::string& message) {
        event_ = message;
        log_.push_back(message);
        if (log_.size() > 5) log_.erase(log_.begin());
    }

    void moveRoom(const std::string& direction) {
        int nextRoom = room_;
        if (direction == "NORTH") {
            if (room_ == 1) nextRoom = 2;
            else if (room_ == 4) nextRoom = 3;
            else if (room_ == 5 && guardianDefeated_) nextRoom = 6;
        } else if (direction == "SOUTH") {
            if (room_ == 2) nextRoom = 1;
            else if (room_ == 3) nextRoom = 4;
            else if (room_ == 6) nextRoom = 5;
        } else if (direction == "EAST") {
            if (room_ == 1) nextRoom = 3;
            else if (room_ == 2) nextRoom = 4;
            else if (room_ == 4) nextRoom = 5;
        } else if (direction == "WEST") {
            if (room_ == 3) nextRoom = 1;
            else if (room_ == 4) nextRoom = 2;
            else if (room_ == 5) nextRoom = 4;
        }

        if (room_ == 5 && direction == "NORTH" && !guardianDefeated_) {
            addLog("The Stone Guardian seals the northern arch. Find and use the Luminous Key.");
            return;
        }
        if (nextRoom == room_) {
            addLog("The ancient walls block this route.");
            return;
        }

        room_ = nextRoom;
        if (direction == "NORTH") player_ = { MAP_X + MAP_W/2, MAP_Y + MAP_H - 46 };
        else if (direction == "SOUTH") player_ = { MAP_X + MAP_W/2, MAP_Y + 46 };
        else if (direction == "EAST") player_ = { MAP_X + 46, MAP_Y + MAP_H/2 };
        else player_ = { MAP_X + MAP_W - 46, MAP_Y + MAP_H/2 };

        addLog(std::string("Entered ") + ROOM_NAMES[room_] + ".");
    }

    std::string interactableItem() const {
        if (room_ == 2 && !hasSeal_) return "ANCIENT SEAL";
        if (room_ == 3 && !hasKey_) return "LUMINOUS KEY";
        if (room_ == 4 && !hasCoin_) return "ROYAL COIN";
        return "";
    }

    Spot itemSpot() const {
        if (room_ == 2) return tileCenter(6, 4);
        if (room_ == 3) return tileCenter(6, 4);
        if (room_ == 4) return tileCenter(8, 4);
        return tileCenter(0, 0);
    }

    Spot characterSpot() const {
        if (room_ == 1) return tileCenter(8, 3);
        if (room_ == 5) return tileCenter(8, 3);
        return tileCenter(0, 0);
    }

    void pickItem(const std::string& item) {
        if (item == "ANCIENT SEAL" && room_ == 2 && !hasSeal_) {
            hasSeal_ = true; score_ += 10;
            addLog("Ancient Seal collected! It is needed in the Hidden Vault.");
        } else if (item == "LUMINOUS KEY" && room_ == 3 && !hasKey_) {
            hasKey_ = true;
            addLog("Luminous Key collected! It can break the Watchtower barrier.");
        } else if (item == "ROYAL COIN" && room_ == 4 && !hasCoin_) {
            hasCoin_ = true; score_ += 10;
            addLog("Royal Coin collected! Bonus treasure: +10 points.");
        } else {
            addLog("That item is not available in this room.");
        }
    }

    void useItem(const std::string& item) {
        if (!hasItem(item)) {
            addLog("You do not have that item in your inventory.");
            return;
        }
        if (item == "LUMINOUS KEY" && room_ == 5 && !guardianDefeated_) {
            guardianDefeated_ = true;
            score_ += 25;
            addLog("The Luminous Key breaks the Guardian's seal. The northern arch opens!");
        } else if (item == "ANCIENT SEAL" && room_ == 6) {
            score_ += 25;
            victory_ = true;
            addLog("The Ancient Seal fits the altar. The Lost Kingdom is restored!");
        } else {
            addLog("The item glows briefly, but nothing changes here.");
        }
    }

    void interact() {
        interactionPulse_ = 0.25f;
        if (room_ == 1 && distanceBetween(player_, characterSpot()) < 92.0f) {
            guideSpoken_ = true;
            addLog("Mira: The Ancient Seal rests in the Library. The Luminous Key lies in the Greenhouse.");
            return;
        }
        if (room_ == 5 && distanceBetween(player_, characterSpot()) < 100.0f) {
            if (hasKey_ && !guardianDefeated_) {
                useItem("LUMINOUS KEY");
            } else if (guardianDefeated_) {
                addLog("The Guardian bows. The path to the Hidden Vault is clear.");
            } else {
                addLog("Guardian: Only the Luminous Key can dissolve this barrier.");
            }
            return;
        }
        const std::string item = interactableItem();
        if (!item.empty() && distanceBetween(player_, itemSpot()) < 92.0f) {
            pickItem(item);
            return;
        }
        if (room_ == 6 && distanceBetween(player_, tileCenter(6, 3)) < 100.0f) {
            useItem("ANCIENT SEAL");
            return;
        }
        addLog("Move closer to a character, object or altar, then press E.");
    }

    void dispatchCommand(const std::string& text) {
        ParsedCommand parsed{};
        if (!parse_command(text.c_str(), &parsed)) {
            addLog("Enter a command after the slash prompt.");
            return;
        }
        const std::string arg(parsed.argument);
        switch (parsed.type) {
            case CMD_HELP:
                addLog("WASD/Arrows move | E interact | I inventory | M map | / command console");
                break;
            case CMD_LOOK:
                addLog(std::string("You are in ") + ROOM_NAMES[room_] + ". Explore and interact with nearby objects.");
                break;
            case CMD_MOVE: moveRoom(arg); break;
            case CMD_PICK: pickItem(arg); break;
            case CMD_USE: useItem(arg); break;
            case CMD_TALK:
                if (room_ == 1) { guideSpoken_ = true; addLog("Mira: Find the seal in the Library and the key in the Greenhouse."); }
                else if (room_ == 5) addLog(guardianDefeated_ ? "The Guardian bows and lets you pass." : "Guardian: Bring me the Luminous Key.");
                else addLog("There is nobody here to talk to.");
                break;
            case CMD_INVENTORY:
                addLog(std::string("Inventory: ") + (hasSeal_ ? "[Ancient Seal] " : "") +
                    (hasKey_ ? "[Luminous Key] " : "") + (hasCoin_ ? "[Royal Coin]" : ""));
                break;
            case CMD_STATUS:
                addLog("Health: 100 | Score: " + std::to_string(score_) + " | Room: " + ROOM_NAMES[room_]);
                break;
            case CMD_MAP: showMap_ = !showMap_; addLog("World map toggled."); break;
            case CMD_DROP: addLog("Dropping quest items is disabled so you cannot lose required objects."); break;
            case CMD_SAVE: saveGame(); break;
            case CMD_LOAD: loadGame(); break;
            case CMD_EXIT: quit_ = true; break;
            case CMD_ATTACK: addLog("The Guardian cannot be defeated by force. Find the Luminous Key."); break;
            default: addLog("Unknown command. Press H for help."); break;
        }
    }

    void saveGame() {
        std::ofstream out("saves/graphical_save.txt");
        if (!out) { addLog("Could not create save file. Run from the project folder."); return; }
        out << room_ << '\n' << score_ << '\n' << hasSeal_ << '\n' << hasKey_
            << '\n' << hasCoin_ << '\n' << guardianDefeated_ << '\n';
        addLog("Progress saved to saves/graphical_save.txt.");
        saveExists_ = true;
    }

    void loadGame() {
        std::ifstream in("saves/graphical_save.txt");
        int room = 1, score = 0;
        bool seal = false, key = false, coin = false, guardian = false;
        if (!(in >> room >> score >> seal >> key >> coin >> guardian)) {
            addLog("No graphical save was found yet. Use SAVE to create one.");
            return;
        }
        room_ = std::clamp(room, 1, 6);
        score_ = score; hasSeal_ = seal; hasKey_ = key; hasCoin_ = coin;
        guardianDefeated_ = guardian;
        victory_ = false;
        player_ = tileCenter(5, 6);
        addLog("Saved progress restored.");
    }

    void update(float dt) {
        animation_ += dt;
        interactionPulse_ = std::max(0.0f, interactionPulse_ - dt);

        if (commandMode_) {
            int ch = GetCharPressed();
            while (ch > 0) {
                if (ch >= 32 && ch <= 126 && commandLine_.size() < 56)
                    commandLine_.push_back(static_cast<char>(ch));
                ch = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE) && !commandLine_.empty()) commandLine_.pop_back();
            if (IsKeyPressed(KEY_ENTER)) {
                dispatchCommand(commandLine_);
                commandLine_.clear();
                commandMode_ = false;
            }
            if (IsKeyPressed(KEY_ESCAPE)) { commandMode_ = false; commandLine_.clear(); }
            return;
        }

        if (IsKeyPressed(KEY_SLASH)) { commandMode_ = true; commandLine_.clear(); return; }
        if (IsKeyPressed(KEY_I)) { showInventory_ = !showInventory_; }
        if (IsKeyPressed(KEY_M)) { showMap_ = !showMap_; }
        if (IsKeyPressed(KEY_H)) dispatchCommand("HELP");
        if (IsKeyPressed(KEY_E)) interact();
        if (IsKeyPressed(KEY_F5)) saveGame();
        if (IsKeyPressed(KEY_F9)) loadGame();

        float dx = 0.0f, dy = 0.0f;
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) dx -= 1.0f;
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) dx += 1.0f;
        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) dy -= 1.0f;
        if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) dy += 1.0f;

        if (dx != 0.0f || dy != 0.0f) {
            const float length = std::sqrt(dx*dx + dy*dy);
            dx /= length; dy /= length;
            const Spot old = player_;
            player_.x += dx * PLAYER_SPEED * dt;
            player_.y += dy * PLAYER_SPEED * dt;
            handleBoundaries(old);
        }
    }

    bool nearDoor(float x, float y, const std::string& direction) const {
        const Spot center = tileCenter(5, 3);
        if (direction == "NORTH" || direction == "SOUTH") return std::fabs(x - center.x) < TILE*1.05f;
        return std::fabs(y - center.y) < TILE*1.05f;
    }

    void handleBoundaries(Spot old) {
        const float minX = MAP_X + 28, maxX = MAP_X + MAP_W - 28;
        const float minY = MAP_Y + 28, maxY = MAP_Y + MAP_H - 28;
        if (player_.y < minY && nearDoor(player_.x, player_.y, "NORTH")) { moveRoom("NORTH"); return; }
        if (player_.y > maxY && nearDoor(player_.x, player_.y, "SOUTH")) { moveRoom("SOUTH"); return; }
        if (player_.x < minX && nearDoor(player_.x, player_.y, "WEST")) { moveRoom("WEST"); return; }
        if (player_.x > maxX && nearDoor(player_.x, player_.y, "EAST")) { moveRoom("EAST"); return; }
        player_.x = std::clamp(player_.x, minX, maxX);
        player_.y = std::clamp(player_.y, minY, maxY);
        (void)old;
    }

    void drawTree(float x, float y) const {
        DrawCircleGradient({x, y + 9}, 26, Color{24, 48, 42, 255}, Color{13, 28, 31, 255});
        DrawCircle(static_cast<int>(x - 7), static_cast<int>(y - 5), 18, Color{67, 132, 86, 255});
        DrawCircle(static_cast<int>(x + 9), static_cast<int>(y - 8), 16, Color{85, 153, 96, 255});
        DrawCircle(static_cast<int>(x), static_cast<int>(y - 17), 14, Color{104, 171, 108, 255});
        DrawRectangle(static_cast<int>(x - 4), static_cast<int>(y + 7), 8, 17, Color{91, 66, 48, 255});
    }

    void drawBookshelf(int col, int row) const {
        const int x = static_cast<int>(MAP_X + col*TILE + 6);
        const int y = static_cast<int>(MAP_Y + row*TILE + 7);
        DrawRectangle(x, y, 52, 48, Color{50, 32, 31, 255});
        DrawRectangle(x + 5, y + 4, 42, 35, Color{102, 65, 47, 255});
        const Color bookColors[] = {
            Color{173, 79, 63, 255}, Color{81, 126, 144, 255},
            Color{204, 161, 83, 255}, Color{89, 135, 91, 255}
        };
        for (int i = 0; i < 5; ++i) DrawRectangle(x + 8 + i*8, y + 9, 6, 26, bookColors[i%4]);
        DrawRectangle(x + 3, y + 39, 46, 5, UI_GOLD);
    }

    void drawRoomDecor() const {
        if (room_ == 1) {
            drawTree(MAP_X + 88, MAP_Y + 95);
            drawTree(MAP_X + 680, MAP_Y + 96);
            drawTree(MAP_X + 95, MAP_Y + 445);
            drawTree(MAP_X + 684, MAP_Y + 445);
            DrawCircle(static_cast<int>(MAP_X + 385), static_cast<int>(MAP_Y + 260), 64, Color{32, 56, 63, 255});
            DrawCircle(static_cast<int>(MAP_X + 385), static_cast<int>(MAP_Y + 253), 52, Color{94, 153, 163, 255});
            DrawCircle(static_cast<int>(MAP_X + 385), static_cast<int>(MAP_Y + 247), 39, Color{44, 91, 112, 255});
            DrawCircle(static_cast<int>(MAP_X + 385), static_cast<int>(MAP_Y + 239), 8, UI_PALE_GOLD);
        } else if (room_ == 2) {
            drawBookshelf(1, 1); drawBookshelf(4, 1); drawBookshelf(7, 1); drawBookshelf(10, 1);
            drawBookshelf(1, 6); drawBookshelf(10, 6);
            DrawRectangle(static_cast<int>(MAP_X + 330), static_cast<int>(MAP_Y + 80), 112, 15, Color{126, 90, 58, 255});
            DrawRectangle(static_cast<int>(MAP_X + 354), static_cast<int>(MAP_Y + 72), 62, 10, UI_GOLD);
        } else if (room_ == 3) {
            for (int c : {1, 4, 8, 10}) {
                DrawRectangle(static_cast<int>(MAP_X + c*TILE + 6), static_cast<int>(MAP_Y + 25), 50, 18, Color{42, 61, 47, 255});
                DrawRectangle(static_cast<int>(MAP_X + c*TILE + 10), static_cast<int>(MAP_Y + 15), 42, 20, Color{92, 161, 94, 255});
                DrawCircle(static_cast<int>(MAP_X + c*TILE + 20), static_cast<int>(MAP_Y + 20), 9, Color{144, 193, 110, 255});
                DrawCircle(static_cast<int>(MAP_X + c*TILE + 39), static_cast<int>(MAP_Y + 18), 9, Color{65, 139, 87, 255});
            }
            for (int c : {1, 10}) {
                DrawRectangle(static_cast<int>(MAP_X + c*TILE + 7), static_cast<int>(MAP_Y + 440), 50, 18, Color{42, 61, 47, 255});
                DrawCircle(static_cast<int>(MAP_X + c*TILE + 22), static_cast<int>(MAP_Y + 440), 14, Color{96, 170, 102, 255});
            }
        } else if (room_ == 4) {
            DrawRectangle(static_cast<int>(MAP_X + TILE), static_cast<int>(MAP_Y + TILE*1.1f), 640, 68, Color{31, 66, 91, 190});
            DrawRectangle(static_cast<int>(MAP_X + TILE), static_cast<int>(MAP_Y + TILE*1.1f), 640, 5, Color{99, 181, 201, 190});
            for (int i = 0; i < 7; ++i)
                DrawLineEx({MAP_X + 100 + i*85.0f, MAP_Y + 150}, {MAP_X + 145 + i*85.0f, MAP_Y + 190}, 2.0f, Color{115, 177, 196, 150});
        } else if (room_ == 5) {
            for (int r : {1, 6}) {
                DrawRectangle(static_cast<int>(MAP_X + 80), static_cast<int>(MAP_Y + r*TILE + 7), 36, 52, Color{52, 52, 72, 255});
                DrawRectangle(static_cast<int>(MAP_X + 650), static_cast<int>(MAP_Y + r*TILE + 7), 36, 52, Color{52, 52, 72, 255});
                DrawRectangle(static_cast<int>(MAP_X + 88), static_cast<int>(MAP_Y + r*TILE + 7), 20, 52, Color{122, 112, 139, 255});
                DrawRectangle(static_cast<int>(MAP_X + 658), static_cast<int>(MAP_Y + r*TILE + 7), 20, 52, Color{122, 112, 139, 255});
            }
        } else {
            DrawCircleGradient({MAP_X + MAP_W/2, MAP_Y + MAP_H/2}, 190,
                               Color{120, 91, 179, 100}, Color{24, 22, 51, 0});
            DrawRectangle(static_cast<int>(MAP_X + 340), static_cast<int>(MAP_Y + 190), 90, 90, Color{58, 42, 92, 255});
            DrawRectangleLinesEx({MAP_X + 340, MAP_Y + 190, 90, 90}, 4, UI_GOLD);
            DrawCircle(static_cast<int>(MAP_X + 385), static_cast<int>(MAP_Y + 235), 24, Color{177, 129, 221, 255});
            DrawCircle(static_cast<int>(MAP_X + 385), static_cast<int>(MAP_Y + 235), 12, UI_PALE_GOLD);
        }
    }

    void drawDoor(const std::string& direction, bool locked = false) const {
        Color c = locked ? UI_RED : UI_GOLD;
        if (direction == "NORTH" || direction == "SOUTH") {
            const float x = MAP_X + MAP_W/2 - 37;
            const float y = direction == "NORTH" ? MAP_Y - 3 : MAP_Y + MAP_H - 15;
            DrawRectangle(static_cast<int>(x), static_cast<int>(y), 74, 28, Color{26, 29, 43, 255});
            DrawRectangleLinesEx({x, y, 74, 28}, 3, c);
            DrawLineEx({x + 12, y + 4}, {x + 12, y + 24}, 3, c);
            DrawLineEx({x + 62, y + 4}, {x + 62, y + 24}, 3, c);
        } else {
            const float x = direction == "WEST" ? MAP_X - 3 : MAP_X + MAP_W - 15;
            const float y = MAP_Y + MAP_H/2 - 37;
            DrawRectangle(static_cast<int>(x), static_cast<int>(y), 28, 74, Color{26, 29, 43, 255});
            DrawRectangleLinesEx({x, y, 28, 74}, 3, c);
            DrawLineEx({x + 4, y + 12}, {x + 24, y + 12}, 3, c);
            DrawLineEx({x + 4, y + 62}, {x + 24, y + 62}, 3, c);
        }
    }

    void drawExits() const {
        if (room_ == 1) { drawDoor("NORTH"); drawDoor("EAST"); }
        if (room_ == 2) { drawDoor("SOUTH"); drawDoor("EAST"); }
        if (room_ == 3) { drawDoor("WEST"); drawDoor("SOUTH"); }
        if (room_ == 4) { drawDoor("WEST"); drawDoor("NORTH"); drawDoor("EAST"); }
        if (room_ == 5) { drawDoor("WEST"); drawDoor("NORTH", !guardianDefeated_); }
        if (room_ == 6) { drawDoor("SOUTH"); }
    }

    void drawItem(Spot s, const std::string& label, Color color) const {
        const float pulse = 4.0f + 2.0f*std::sin(animation_*3.0f);
        DrawCircleGradient({s.x, s.y}, 29 + pulse, Fade(color, 0.5f), Fade(color, 0.0f));
        DrawCircle(static_cast<int>(s.x), static_cast<int>(s.y), 15, color);
        DrawPoly({s.x, s.y - 1}, 4, 15, 45.0f, UI_PALE_GOLD);
        DrawCircle(static_cast<int>(s.x), static_cast<int>(s.y), 4, Color{255, 255, 255, 255});
        DrawText(label.c_str(), static_cast<int>(s.x - MeasureText(label.c_str(), 12)/2), static_cast<int>(s.y + 23), 12, UI_TEXT);
    }

    void drawCharacter(Spot s, bool guardian) const {
        Color mainColor = guardian ? Color{127, 129, 153, 255} : Color{98, 172, 191, 255};
        DrawEllipse(static_cast<int>(s.x), static_cast<int>(s.y + 19), 23, 8, Color{10, 16, 25, 120});
        DrawRectangleRounded({s.x - 18, s.y - 2, 36, 38}, 0.28f, 6, mainColor);
        DrawRectangleRounded({s.x - 13, s.y + 4, 26, 28}, 0.18f, 5, guardian ? Color{75, 78, 101, 255} : Color{73, 108, 157, 255});
        DrawCircle(static_cast<int>(s.x), static_cast<int>(s.y - 12), 16, guardian ? Color{161, 164, 183, 255} : Color{226, 186, 142, 255});
        DrawRectangle(static_cast<int>(s.x - 16), static_cast<int>(s.y - 22), 32, 8, guardian ? Color{79, 83, 104, 255} : Color{91, 62, 47, 255});
        DrawCircle(static_cast<int>(s.x - 5), static_cast<int>(s.y - 12), 2, UI_INK);
        DrawCircle(static_cast<int>(s.x + 5), static_cast<int>(s.y - 12), 2, UI_INK);
        DrawCircleLines(static_cast<int>(s.x), static_cast<int>(s.y - 12), 16, Fade(UI_GOLD, 0.7f));
        const char* name = guardian ? "STONE GUARDIAN" : "MIRA";
        DrawText(name, static_cast<int>(s.x - MeasureText(name, 12)/2), static_cast<int>(s.y + 44), 12, UI_PALE_GOLD);
    }

    void drawPlayer() const {
        const float bob = std::sin(animation_*8.0f) * 2.0f;
        DrawEllipse(static_cast<int>(player_.x), static_cast<int>(player_.y + 20), 17, 7, Color{9, 14, 22, 130});
        DrawRectangleRounded({player_.x - 13, player_.y - 3 + bob, 26, 30}, 0.25f, 5, Color{58, 116, 190, 255});
        DrawRectangleRounded({player_.x - 9, player_.y + 3 + bob, 18, 19}, 0.2f, 5, Color{42, 74, 137, 255});
        DrawCircle(static_cast<int>(player_.x), static_cast<int>(player_.y - 12 + bob), 12, Color{231, 187, 145, 255});
        DrawRectangle(static_cast<int>(player_.x - 12), static_cast<int>(player_.y - 21 + bob), 24, 7, Color{52, 38, 43, 255});
        DrawCircle(static_cast<int>(player_.x - 4), static_cast<int>(player_.y - 12 + bob), 2, UI_INK);
        DrawCircle(static_cast<int>(player_.x + 4), static_cast<int>(player_.y - 12 + bob), 2, UI_INK);
        DrawCircleLines(static_cast<int>(player_.x), static_cast<int>(player_.y - 12 + bob), 13, UI_PALE_GOLD);
    }

    void drawWorld() const {
        DrawRectangleRounded({24, 118, 810, 620}, 0.04f, 8, UI_PANEL);
        DrawText(ROOM_NAMES[room_], 44, 128, 20, UI_TEXT);
        DrawText("EXPLORE THE KINGDOM", 650, 133, 10, UI_MUTED);
        for (int row = 0; row < ROWS; ++row) {
            for (int col = 0; col < COLS; ++col) {
                const int x = static_cast<int>(MAP_X + col*TILE);
                const int y = static_cast<int>(MAP_Y + row*TILE);
                DrawRectangle(x, y, static_cast<int>(TILE), static_cast<int>(TILE), floorColor(room_, col, row));
                DrawRectangleLines(x, y, static_cast<int>(TILE), static_cast<int>(TILE), Fade(Color{12, 18, 28, 255}, 0.18f));
            }
        }
        DrawRectangleLinesEx({MAP_X, MAP_Y, MAP_W, MAP_H}, 7, Color{21, 28, 42, 255});
        drawRoomDecor();
        drawExits();

        if (room_ == 1) drawCharacter(characterSpot(), false);
        if (room_ == 5 && !guardianDefeated_) drawCharacter(characterSpot(), true);
        const std::string item = interactableItem();
        if (!item.empty()) {
            Color c = item == "LUMINOUS KEY" ? Color{89, 204, 230, 255} :
                      item == "ANCIENT SEAL" ? Color{232, 182, 77, 255} : Color{202, 191, 135, 255};
            drawItem(itemSpot(), item == "LUMINOUS KEY" ? "KEY" : item == "ANCIENT SEAL" ? "SEAL" : "COIN", c);
        }
        if (room_ == 6) drawItem(tileCenter(6, 3), "ALTAR", Color{172, 126, 230, 255});
        drawPlayer();

        if (showMap_) drawMapOverlay();
        if (showInventory_) drawInventoryOverlay();
    }

    void drawMapOverlay() const {
        DrawRectangleRounded({260, 270, 430, 210}, 0.08f, 10, Fade(UI_INK, 0.96f));
        DrawRectangleLinesEx({260, 270, 430, 210}, 2, UI_GOLD);
        DrawText("ELDORIA  /  WORLD MAP", 286, 286, 19, UI_PALE_GOLD);
        const char* names[] = {"1 Courtyard", "2 Library", "3 Greenhouse", "4 Sunken Hall", "5 Watchtower", "6 Hidden Vault"};
        for (int i = 0; i < 6; ++i) {
            Color c = (i + 1 == room_) ? UI_GOLD : UI_MUTED;
            DrawCircle(294, 334 + i*23, 5, c);
            DrawText(names[i], 310, 326 + i*23, 15, c);
        }
        DrawText("Press M to close", 540, 449, 11, UI_MUTED);
    }

    void drawInventoryOverlay() const {
        DrawRectangleRounded({240, 285, 470, 190}, 0.08f, 10, Fade(UI_INK, 0.97f));
        DrawRectangleLinesEx({240, 285, 470, 190}, 2, UI_GOLD);
        DrawText("INVENTORY", 268, 302, 20, UI_PALE_GOLD);
        int y = 344;
        if (!hasSeal_ && !hasKey_ && !hasCoin_) {
            DrawText("Your satchel is empty. Explore to find useful objects.", 268, y, 15, UI_MUTED);
        } else {
            if (hasSeal_) { DrawCircle(278, y+8, 8, UI_GOLD); DrawText("ANCIENT SEAL  /  required for the final vault", 297, y, 14, UI_TEXT); y += 31; }
            if (hasKey_) { DrawCircle(278, y+8, 8, SKYBLUE); DrawText("LUMINOUS KEY  /  opens the Guardian barrier", 297, y, 14, UI_TEXT); y += 31; }
            if (hasCoin_) { DrawCircle(278, y+8, 8, UI_PALE_GOLD); DrawText("ROYAL COIN  /  optional treasure", 297, y, 14, UI_TEXT); y += 31; }
        }
        DrawText("Press I to close", 560, 447, 11, UI_MUTED);
    }

    void drawHeader() const {
        DrawRectangleGradientV(0, 0, SCREEN_W, 100, Color{13, 19, 34, 255}, Color{28, 31, 54, 255});
        DrawText("THE LOST KINGDOM", 28, 21, 30, UI_PALE_GOLD);
        DrawText("A 2D FANTASY ADVENTURE", 31, 58, 11, UI_MUTED);
        DrawText("ELDORIA  /  QUEST STATUS", 730, 20, 11, UI_MUTED);
        DrawText(TextFormat("SCORE  %03i", score_), 730, 44, 18, UI_GOLD);
        DrawText("HEALTH", 930, 22, 10, UI_MUTED);
        DrawRectangleRounded({930, 43, 190, 13}, 0.5f, 8, Color{51, 59, 77, 255});
        DrawRectangleRounded({930, 43, 190.0f * health_/100.0f, 13}, 0.5f, 8, UI_GREEN);
        DrawText(TextFormat("%i / 100", health_), 1130, 41, 13, UI_TEXT);
        DrawLine(24, 98, SCREEN_W - 24, 98, Fade(UI_GOLD, 0.45f));
    }

    void drawSidePanel() const {
        DrawRectangleRounded({854, 118, 402, 620}, 0.04f, 8, UI_PANEL);
        DrawText("QUEST OBJECTIVES", 878, 140, 17, UI_PALE_GOLD);
        DrawLine(878, 168, 1230, 168, Fade(UI_GOLD, 0.35f));

        auto objective = [&](int y, const char* label, bool done) {
            DrawCircle(891, y + 8, 7, done ? UI_GREEN : Color{74, 86, 108, 255});
            if (done) { DrawLineEx({887.0f, static_cast<float>(y+8)}, {890.0f, static_cast<float>(y+11)}, 2.0f, UI_TEXT); DrawLineEx({890.0f, static_cast<float>(y+11)}, {896.0f, static_cast<float>(y+4)}, 2.0f, UI_TEXT); }
            DrawText(label, 910, y, 14, done ? Color{150, 202, 166, 255} : UI_TEXT);
        };
        objective(187, "Speak to Mira in the Courtyard", guideSpoken_);
        objective(218, "Find the Ancient Seal", hasSeal_);
        objective(249, "Find the Luminous Key", hasKey_);
        objective(280, "Open the Watchtower barrier", guardianDefeated_);
        objective(311, "Restore the Hidden Vault", victory_);

        DrawLine(878, 350, 1230, 350, Fade(UI_GOLD, 0.35f));
        DrawText("YOUR SATCHEL", 878, 370, 17, UI_PALE_GOLD);
        int y = 402;
        if (!hasSeal_ && !hasKey_ && !hasCoin_) {
            DrawText("No items collected yet.", 880, y, 13, UI_MUTED);
        } else {
            if (hasSeal_) { DrawCircle(889, y+8, 7, UI_GOLD); DrawText("Ancient Seal", 906, y, 13, UI_TEXT); y += 27; }
            if (hasKey_) { DrawCircle(889, y+8, 7, SKYBLUE); DrawText("Luminous Key", 906, y, 13, UI_TEXT); y += 27; }
            if (hasCoin_) { DrawCircle(889, y+8, 7, UI_PALE_GOLD); DrawText("Royal Coin", 906, y, 13, UI_TEXT); y += 27; }
        }

        DrawLine(878, 500, 1230, 500, Fade(UI_GOLD, 0.35f));
        DrawText("KINGDOM JOURNAL", 878, 519, 17, UI_PALE_GOLD);
        int logY = 551;
        for (const auto& entry : log_) {
            const int maxW = 345;
            std::string line = entry;
            while (!line.empty()) {
                std::string shown = line;
                while (MeasureText(shown.c_str(), 12) > maxW && shown.size() > 8)
                    shown.pop_back();
                if (shown.size() < line.size()) {
                    const auto split = shown.find_last_of(' ');
                    if (split != std::string::npos) shown.resize(split);
                }
                DrawText(shown.c_str(), 880, logY, 12, UI_MUTED);
                logY += 16;
                if (shown.size() >= line.size()) break;
                line.erase(0, shown.size());
                while (!line.empty() && line.front() == ' ') line.erase(line.begin());
            }
            logY += 4;
            if (logY > 710) break;
        }
    }

    void drawBottomBar() const {
        DrawRectangleRounded({24, 751, 1232, 35}, 0.25f, 8, UI_PANEL2);
        DrawText("WASD / ARROWS", 42, 762, 12, UI_PALE_GOLD);
        DrawText("MOVE", 153, 762, 11, UI_MUTED);
        DrawText("E", 215, 762, 12, UI_PALE_GOLD);
        DrawText("INTERACT", 232, 762, 11, UI_MUTED);
        DrawText("I", 315, 762, 12, UI_PALE_GOLD);
        DrawText("INVENTORY", 331, 762, 11, UI_MUTED);
        DrawText("M", 430, 762, 12, UI_PALE_GOLD);
        DrawText("MAP", 446, 762, 11, UI_MUTED);
        DrawText("/", 500, 762, 12, UI_PALE_GOLD);
        DrawText("COMMANDS", 514, 762, 11, UI_MUTED);
        DrawText("F5 SAVE  /  F9 LOAD  /  H HELP", 1000, 762, 11, UI_MUTED);
    }

    void drawCommandOverlay() const {
        DrawRectangle(0, 0, SCREEN_W, SCREEN_H, Fade(UI_INK, 0.78f));
        DrawRectangleRounded({245, 310, 790, 160}, 0.06f, 10, UI_PANEL);
        DrawRectangleLinesEx({245, 310, 790, 160}, 2, UI_GOLD);
        DrawText("COMMAND CONSOLE  /  C PARSER", 275, 333, 19, UI_PALE_GOLD);
        DrawRectangleRounded({275, 375, 730, 42}, 0.15f, 8, Color{11, 16, 28, 255});
        DrawText(">", 292, 386, 18, UI_GOLD);
        DrawText(commandLine_.c_str(), 318, 386, 17, UI_TEXT);
        DrawText("ENTER to execute  |  BACKSPACE to edit  |  ESC to close", 275, 435, 12, UI_MUTED);
    }

    void drawStartScreen() const {
        DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H, Color{12, 18, 34, 255}, Color{31, 29, 58, 255});
        for (int i = 0; i < 70; ++i) {
            const int x = (i*193 + 41) % SCREEN_W;
            const int y = (i*79 + 28) % SCREEN_H;
            DrawCircle(x, y, (i%3 == 0) ? 2 : 1, Fade(UI_PALE_GOLD, 0.45f));
        }
        DrawCircleGradient({640, 340}, 190, Fade(UI_PURPLE, 0.38f), Fade(UI_PURPLE, 0.0f));
        DrawPoly({640, 320}, 6, 92, 30, Fade(UI_GOLD, 0.85f));
        DrawPoly({640, 320}, 6, 70, 30, Color{36, 32, 64, 255});
        DrawCircle(640, 320, 34, Color{232, 182, 92, 255});
        DrawCircle(640, 320, 21, Color{80, 109, 150, 255});
        DrawText("THE LOST KINGDOM", 0, 485, 48, UI_PALE_GOLD);
        DrawText("A GRAPHICAL ADVENTURE THROUGH FORGOTTEN ELDORIA", 0, 544, 16, UI_MUTED);
        const int titleW = MeasureText("THE LOST KINGDOM", 48);
        DrawText("THE LOST KINGDOM", (SCREEN_W-titleW)/2, 485, 48, UI_PALE_GOLD);
        const char* subtitle = "A GRAPHICAL ADVENTURE THROUGH FORGOTTEN ELDORIA";
        DrawText(subtitle, (SCREEN_W-MeasureText(subtitle, 16))/2, 544, 16, UI_MUTED);
        const char* prompt = "PRESS ENTER TO BEGIN";
        DrawText(prompt, (SCREEN_W-MeasureText(prompt, 18))/2, 650, 18, UI_TEXT);
        DrawText("WASD / ARROWS  MOVE     E  INTERACT     I  INVENTORY", 0, 695, 12, UI_MUTED);
        DrawText("C11 + C++17  |  POWERED BY RAYLIB", 0, 744, 11, UI_GOLD);
    }

    void drawVictoryScreen() const {
        DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H, Color{12, 18, 34, 255}, Color{39, 34, 75, 255});
        for (int i = 0; i < 80; ++i) {
            const int x = (i*173 + 17) % SCREEN_W;
            const int y = (i*97 + 12) % SCREEN_H;
            DrawCircle(x, y, (i%2 == 0) ? 3 : 2, Fade(UI_PALE_GOLD, 0.7f));
        }
        DrawCircleGradient({640, 270}, 210, Fade(UI_GOLD, 0.35f), Fade(UI_GOLD, 0.0f));
        DrawPoly({640, 250}, 8, 92, 22.5f, UI_GOLD);
        DrawPoly({640, 250}, 8, 67, 22.5f, Color{80, 80, 135, 255});
        DrawCircle(640, 250, 31, UI_PALE_GOLD);
        DrawText("ELDORIA IS RESTORED", (SCREEN_W-MeasureText("ELDORIA IS RESTORED", 38))/2, 430, 38, UI_PALE_GOLD);
        const char* message = "The Ancient Seal has awakened the heart of the kingdom.";
        DrawText(message, (SCREEN_W-MeasureText(message, 17))/2, 492, 17, UI_TEXT);
        DrawText(TextFormat("FINAL SCORE  %03i", score_), (SCREEN_W-MeasureText(TextFormat("FINAL SCORE  %03i", score_), 20))/2, 548, 20, UI_GOLD);
        const char* close = "PRESS ESC TO CLOSE";
        DrawText(close, (SCREEN_W-MeasureText(close, 13))/2, 650, 13, UI_MUTED);
    }

    void draw() const {
        if (startScreen_) { drawStartScreen(); return; }
        if (victory_) { drawVictoryScreen(); return; }
        ClearBackground(UI_INK);
        drawHeader();
        drawWorld();
        drawSidePanel();
        drawBottomBar();
        if (commandMode_) drawCommandOverlay();
    }
};
} // namespace

int VisualGame::run() {
    LostKingdomWindow game;
    return game.run();
}
