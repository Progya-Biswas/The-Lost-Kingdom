<div align="center">

# ✦ THE LOST KINGDOM ✦
### A 2D FANTASY ADVENTURE BUILT WITH C11 + C++17

**EXPLORE  ·  COLLECT  ·  UNLOCK  ·  RESTORE**

![C11](https://img.shields.io/badge/C-C11-00599C?style=for-the-badge&logo=c&logoColor=white)
![C++17](https://img.shields.io/badge/C++-17-7B2CBF?style=for-the-badge&logo=cplusplus&logoColor=white)
![raylib](https://img.shields.io/badge/Graphics-raylib%206.2-9A72CF?style=for-the-badge)
![CMake](https://img.shields.io/badge/Build-CMake-064F8C?style=for-the-badge&logo=cmake&logoColor=white)

*A small kingdom, six connected locations, two essential relics, and one final vault.*

</div>

---

## 01 / THE GAME

**The Lost Kingdom** is a 2D top-down fantasy exploration game. The graphical edition replaces the original command-only interface with a desktop window, animated player character, illustrated rooms, collectible objects, doors, NPCs, a quest tracker, and an inventory panel.

The player explores Eldoria, meets Mira the Guide, finds the Ancient Seal in the Old Library, retrieves the Luminous Key from the Greenhouse, opens the Stone Guardian's barrier in the Watchtower, and restores the Hidden Vault.

The original terminal edition is still available as an optional executable for studying the C command parser and the C++ game model.

## 02 / GRAPHICAL FEATURES

- **Six connected rooms:** Arrival Courtyard, Old Library, Greenhouse, Sunken Hall, Watchtower and Hidden Vault.
- **Top-down movement:** walk using WASD or the arrow keys.
- **NPC interaction:** speak to Mira and the Stone Guardian.
- **Collectibles:** Ancient Seal, Luminous Key and optional Royal Coin.
- **Quest tracker:** live objectives and collected items.
- **World map and inventory overlays:** press M and I.
- **In-game command console:** press / to type commands; the C11 parser interprets them.
- **Save and load:** F5 saves and F9 restores progress.
- **Victory scene:** restore the seal in the Hidden Vault.

## 03 / HOW TO PLAY

| Key | Action |
|---|---|
| W A S D / Arrow keys | Move the character |
| E | Interact with a nearby character, item or altar |
| I | Open or close inventory |
| M | Open or close the world map |
| H | Show help |
| / | Open the typed command console |
| Enter | Execute a command in the command console |
| Backspace | Edit a command |
| F5 / F9 | Save / load progress |
| Esc | Close an overlay or exit the window |

### Recommended winning route

1. Start in the Arrival Courtyard and walk near Mira. Press **E** to hear her clue.
2. Go through the northern door to the Old Library. Walk near the glowing Ancient Seal and press **E**.
3. Return to the Courtyard and take the eastern door to the Greenhouse.
4. Collect the Luminous Key with **E**.
5. Travel south to the Sunken Hall, then east to the Watchtower.
6. Walk near the Stone Guardian and press **E**. The key opens the northern arch.
7. Enter the Hidden Vault, walk up to the altar, and press **E** while carrying the Ancient Seal.

You can also press **/** and type commands such as `TALK`, `MOVE NORTH`, `PICK ANCIENT SEAL`, or `USE LUMINOUS KEY`.

## 04 / BUILD AND RUN ON WINDOWS

### Requirements
- Windows 10 or 11, 64-bit
- Visual Studio 2022 with **Desktop development with C++**
- CMake (included with the Visual Studio C++ workload)
- Internet access on the first CMake configure so CMake can fetch raylib 6.2

### Step 1 — download the project
Open the repository, choose **Code → Download ZIP**, extract the ZIP, and open **Developer PowerShell for Visual Studio 2022**.

### Step 2 — move into the project folder

```powershell
cd "$HOME\Downloads\The-Lost-Kingdom-main"
```

Adjust the folder name if Windows extracted it with a different suffix.

### Step 3 — configure and build

```powershell
cmake -S . -B build
cmake --build build --config Release
```

The first configure downloads and builds the pinned raylib dependency. Later builds reuse the downloaded dependency.

### Step 4 — launch the graphical game

```powershell
.\build\Release\lost_kingdom.exe
```

A resizable game window opens. Press **Enter** on the title screen to begin.

### Optional — launch the original terminal edition

```powershell
.\build\Release\lost_kingdom_cli.exe
```

### MinGW alternative

When using MinGW-w64, configure with the MinGW Makefiles generator:

```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
.\build\lost_kingdom.exe
```

## 05 / C AND C++ CONCEPTS

### C11 utility layer
- `enum CommandType` gives each supported command a named constant.
- `struct ParsedCommand` stores the command type and argument together.
- Pointers and `const char*` are used for input and output parameters.
- `snprintf`, `strtok`, `toupper` and `strcmp` implement bounded command parsing and case normalisation.
- `malloc`, `memcpy` and `free` demonstrate dynamic memory ownership.
- `calculate_score()` demonstrates functions, conditions, arithmetic and return values.

### C++17 game model and graphical layer
- **Encapsulation:** Player, Room and Game keep their state private and expose methods.
- **Inheritance:** Treasure and Tool derive from Item; Guide and Guardian derive from Character.
- **Virtual functions:** Item behaviour is selected through base-class pointers.
- **Abstract class:** Character declares the pure virtual `interact()` interface.
- **Operator overloading:** Item overloads `==` to compare names.
- **STL:** `std::map`, `std::vector`, `std::find_if` and lambdas manage the game data.
- **Smart pointers:** `std::shared_ptr` manages item and character lifetimes.
- **File handling:** `std::ofstream` and `std::ifstream` implement save/load.
- **Exception handling:** C++ exceptions are caught around command execution and startup.
- **Graphics and event loop:** the raylib window loop reads keyboard input, updates the player, and draws each frame.

### How C and C++ are integrated

The graphical application creates an uppercase command string for an action and passes it to the C function `parse_command()`. The C parser fills a `ParsedCommand` structure. The C++ graphical controller dispatches the parsed enum to the corresponding game action. The command console exposes the same parser to the player.

The C header uses `extern "C"` when included by C++, which preserves C language linkage. CMake compiles the C module as C11 and the game modules as C++17, then links them into the application.

## 06 / PROJECT STRUCTURE

```text
The-Lost-Kingdom/
├── .github/workflows/build.yml
├── c_modules/c_utils.c
├── cpp_modules/Game.cpp
├── data/rooms.txt
├── data/story.txt
├── docs/
│   ├── GAME_DESIGN.md
│   ├── GAMEPLAY_TRANSCRIPT.md
│   ├── GRAPHICAL_GAME_GUIDE.md
│   └── The_Lost_Kingdom_Project_Report.docx
├── include/
│   ├── Game.hpp
│   └── c_utils.h
├── tests/test_command_parser.c
├── visual/
│   ├── VisualGame.cpp
│   └── VisualGame.hpp
├── cli_main.cpp
├── main.cpp
├── CMakeLists.txt
└── README.md
```

## 07 / DOCUMENTATION

- [Graphical game setup and walkthrough](docs/GRAPHICAL_GAME_GUIDE.md)
- [Game design and technical notes](docs/GAME_DESIGN.md)
- [Original command-line gameplay transcript](docs/GAMEPLAY_TRANSCRIPT.md)
- [C/C++ project report (Word)](docs/The_Lost_Kingdom_Project_Report.docx)

## 08 / TESTING AND CI

The C command parser is tested with CTest. GitHub Actions configures the project, builds the graphical and command-line targets, and runs the parser tests.

```bash
ctest --test-dir build --output-on-failure
```

## 09 / KNOWN SCOPE

The room layouts and visual objects are currently defined in code. The text files under `data/` are reference data and are not dynamically loaded yet. The game uses raylib drawing primitives for its initial art, so it does not require separate sprite downloads. The save file is a simple educational format, not a versioned commercial save system.

---

<div align="center">

**THE LOST KINGDOM**  
*Built with C. Modelled with C++. Brought to life with raylib.*

</div>