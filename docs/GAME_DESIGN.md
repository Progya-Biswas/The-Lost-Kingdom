# The Lost Kingdom — Game Design & Technical Notes

## 1. Overview
The Lost Kingdom is a terminal-based exploration adventure written in C and C++17. The player explores six connected locations, talks to characters, collects objects, unlocks passages, solves a final puzzle, and tries to restore the kingdom's seal.

## 2. World and progression

| ID | Location | Role |
|---:|---|---|
| 1 | Arrival Courtyard | Starting point; meet Mira the Guide |
| 2 | Old Library | Find the Ancient Seal |
| 3 | Greenhouse | Find the Luminous Key |
| 4 | Sunken Hall | Connects the eastern route to the Watchtower |
| 5 | Watchtower | Face the Stone Guardian; use the Luminous Key |
| 6 | Hidden Vault | Solve the final seal puzzle |

A typical route is Courtyard → Library → Courtyard → Greenhouse → Sunken Hall → Watchtower → Hidden Vault. The key and seal are required to complete the progression.

## 3. Commands
HELP, LOOK, MOVE <direction>, PICK <item>, DROP <item>, INVENTORY, USE <item>, TALK, ATTACK, STATUS, MAP, SAVE, LOAD, and EXIT. Commands are parsed by the C utility module and dispatched by the C++ game loop.

## 4. Architecture
- main.cpp: exception-safe program entry point.
- include/Game.hpp: domain model and public/private class interfaces.
- include/c_utils.h: C-compatible command and utility API.
- c_modules/c_utils.c: command parser, score calculation, and C string duplication helper.
- cpp_modules/Game.cpp: items, characters, rooms, player inventory, world setup, game loop, puzzles, and save/load.
- data/: editable reference descriptions of rooms and story beats. Current world creation is performed in code; these files are not yet loaded dynamically.

## 5. C/C++ integration
The C source is compiled as C11 into a static library. Its header wraps function declarations in extern "C" when included from C++, ensuring compatible linkage. The main game is compiled as C++17 and links against the C utility library.

## 6. Object-oriented concepts
The project demonstrates encapsulation through Player, Room, and Game; inheritance through Treasure and Tool deriving from Item; abstract interfaces and virtual dispatch through Character, Guide, and Guardian; smart pointers for shared item/character ownership; and STL containers (map, vector) for world and inventory state.

## 7. Build and run
See the root README for CMake and direct compiler commands. Save/load uses saves/savegame.txt, relative to the current working directory.

## 8. Limitations and future work
The room and story text files are reference content, not runtime-loaded data. Save files use a compact demonstration format. Future iterations could add automated tests, robust versioned save files, configurable world data, accessibility settings, richer combat, and a graphical front end.