# The Lost Kingdom — Graphical Game Guide

## First launch
1. Download the repository ZIP and extract it.
2. Open Developer PowerShell for Visual Studio 2022 in the extracted project directory.
3. Run `cmake -S . -B build`.
4. Run `cmake --build build --config Release`.
5. Start `.\build\Release\lost_kingdom.exe`.
6. Press Enter on the title screen.

The first CMake configure downloads raylib 6.2. Internet access is required for that first configure.

## Screen layout
- The large left viewport renders the current room as a top-down tile scene.
- The character is controlled with WASD or arrow keys.
- The panel on the right lists objectives, inventory and recent story messages.
- Doors are shown in gold. The Watchtower's locked north exit is red until the key is used.
- The top bar shows the current score and health.
- The bottom bar displays the keyboard controls.

## Controls
| Key | Action |
|---|---|
| WASD / arrows | Walk |
| E | Interact with a nearby object or character |
| I | Inventory overlay |
| M | World map overlay |
| H | Help |
| / | Typed command console using the C parser |
| Enter | Execute typed command |
| Backspace | Delete a character in typed command |
| F5 / F9 | Save / load |
| Esc | Close the game window |

## Successful route
1. Move close to Mira in the Arrival Courtyard and press E.
2. Go north to the Old Library. Move near the glowing seal and press E.
3. Return south, then travel east to the Greenhouse. Collect the Luminous Key with E.
4. Travel south into the Sunken Hall, then east to the Watchtower.
5. Move close to the Stone Guardian and press E. The carried key dissolves its barrier.
6. Move north to the Hidden Vault, approach the altar and press E while carrying the Ancient Seal.

An optional Royal Coin in the Sunken Hall gives bonus points.

## Typed command console
Press / to open the in-game console. Enter commands such as:
```text
HELP
LOOK
TALK
MOVE NORTH
PICK ANCIENT SEAL
USE LUMINOUS KEY
INVENTORY
STATUS
MAP
SAVE
LOAD
```

The input is parsed by `parse_command()` in the C11 module. The graphical game then dispatches the resulting `CommandType` in C++.

## Build targets
- `lost_kingdom`: graphical raylib game (default).
- `lost_kingdom_cli`: optional original terminal version.
- `test_command_parser`: C utility/parser tests (CTest).

## Notes
The game art is drawn with raylib primitives in code, so no extra sprite package needs to be installed. Save data is stored relative to the project folder under `saves/graphical_save.txt`.
