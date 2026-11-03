<div align="center">

# ◈ THE LOST KINGDOM ◈
### A TERMINAL ADVENTURE FROM THE EDGE OF THE UNKNOWN

**[ EXPLORE ] · [ COLLECT ] · [ DECODE ] · [ UNLOCK ]**

*A C + C++ console game built around exploration, puzzles, inventory systems and object-oriented design.*

![C](https://img.shields.io/badge/C-C11-00599C?style=for-the-badge&logo=c&logoColor=white)
![C++](https://img.shields.io/badge/C++-17-7B2CBF?style=for-the-badge&logo=cplusplus&logoColor=white)
![Build](https://img.shields.io/badge/Build-CMake-064F8C?style=for-the-badge&logo=cmake&logoColor=white)
![Platform](https://img.shields.io/badge/Platform-Console-00B4D8?style=for-the-badge)

</div>

---

## `01 / SYSTEM BRIEF`

A forgotten kingdom. Six connected locations. A guardian at the northern arch. One ancient seal that can reveal the truth.

**The Lost Kingdom** is a text-driven adventure in which the player explores an interconnected world, gathers objects, interacts with characters and solves a final puzzle. The project demonstrates how procedural C utilities can work alongside an object-oriented C++ game engine.

> **MISSION DIRECTIVE**  
> Recover the Ancient Seal. Find the Luminous Key. Open the Watchtower passage. Reach the Hidden Vault and restore the kingdom's final seal.

## `02 / GAMEPLAY PREVIEW`

A successful playthrough follows this route:
1. Start in the **Arrival Courtyard** and talk to Mira the Guide.
2. Travel north to the **Old Library** and collect the `ANCIENT SEAL`.
3. Return to the courtyard, travel east to the **Greenhouse**, and collect the `LUMINOUS KEY`.
4. Travel south to the **Sunken Hall**, then east to the **Watchtower**.
5. Use the `LUMINOUS KEY` to dissolve the guardian's seal.
6. Move north into the **Hidden Vault** and use the `ANCIENT SEAL`.

### Gameplay capture

![The Lost Kingdom gameplay](screenshots/gameplay_screenshot.jpg)

A text transcript is available at [screenshots/gameplay_transcript.txt](screenshots/gameplay_transcript.txt).

## `03 / WORLD MAP`

```text
                 ┌───────────────────┐
                 │  2  OLD LIBRARY   │
                 │   ANCIENT SEAL    │
                 └─────────┬─────────┘
                           │ SOUTH
┌───────────────────┐      │      ┌───────────────────┐
│ 1 ARRIVAL         │ EAST │      │ 3 GREENHOUSE      │
│   COURTYARD       ├──────┼──────┤   LUMINOUS KEY    │
│   MIRA THE GUIDE  │      │      └─────────┬─────────┘
└───────────────────┘      │                │ SOUTH
                           ▼                ▼
                    ┌──────────────────────────┐
                    │       4 SUNKEN HALL      │
                    └────────────┬─────────────┘
                                 │ EAST
                                 ▼
                    ┌──────────────────────────┐
                    │      5 WATCHTOWER        │
                    │     STONE GUARDIAN       │
                    └────────────┬─────────────┘
                                 │ NORTH
                           KEY REQUIRED
                                 │
                                 ▼
                    ┌──────────────────────────┐
                    │       6 HIDDEN VAULT     │
                    │    FINAL SEAL PUZZLE     │
                    └──────────────────────────┘
```

## `04 / COMMAND CONSOLE`

| Command | Function |
|---|---|
| `HELP` | Display command reference |
| `LOOK` | Inspect the current location |
| `MOVE NORTH` | Travel through an available exit |
| `PICK <ITEM>` | Collect an object |
| `DROP <ITEM>` | Drop an object |
| `INVENTORY` | Inspect carried objects |
| `USE <ITEM>` | Use an object on a puzzle |
| `TALK` | Interact with a character |
| `ATTACK` | Attempt an attack |
| `STATUS` | Display health, score and location |
| `MAP` | Display a simplified map |
| `SAVE` / `LOAD` | Save or restore progress |
| `EXIT` | Leave the game |

Commands are case-insensitive. Use item names as shown by the room and inventory descriptions.

## `05 / BUILD & LAUNCH`

### Requirements
- CMake 3.16 or newer
- GCC/G++ or Clang with C11 and C++17 support, or MSVC

### Build with CMake

```bash
cmake -S . -B build
cmake --build build
```

Run the executable from the repository root:

```bash
./build/lost_kingdom
```

On Windows, run `build\Debug\lost_kingdom.exe` for a Visual Studio generator, or `build\lost_kingdom.exe` for MinGW/Makefiles.

### Compile manually with GCC/G++

```bash
mkdir -p build
gcc -std=c11 -Wall -Wextra -Wpedantic -Iinclude -c c_modules/c_utils.c -o build/c_utils.o
g++ -std=c++17 -Wall -Wextra -Wpedantic -Iinclude main.cpp cpp_modules/Game.cpp build/c_utils.o -o build/lost_kingdom
./build/lost_kingdom
```

## `06 / C + C++ ENGINEERING`

| Area | Concepts applied |
|---|---|
| C foundation | Data types, operators, conditions, loops, functions |
| C data handling | Arrays, strings, `struct`, `enum`, pointers |
| C memory | `malloc`, `free` ownership contract |
| C/C++ interface | Header guards and `extern "C"` linkage |
| C++ object model | Classes, objects, encapsulation, constructors, destructors |
| OOP relationships | Inheritance, abstract classes, virtual functions, polymorphism |
| Generic programming | Standard library containers and algorithms |
| Operator design | `Item::operator==` |
| STL | `vector`, `map`, `shared_ptr` |
| Reliability | File streams, exceptions, validation |

### Architecture

```text
The-Lost-Kingdom/
├── include/       # C and C++ interfaces
├── c_modules/     # C command parsing and utility functions
├── cpp_modules/   # C++ game engine and world model
├── data/          # Room and story reference data
├── docs/          # Design notes, report and gameplay guide
├── screenshots/   # Gameplay image and transcript
├── tests/         # C command-parser tests
├── saves/         # Runtime save location
├── main.cpp
├── CMakeLists.txt
└── README.md
```

**C layer:** command parsing and reusable utility functions.  
**C++ layer:** game world, player, items, characters, rooms, and progression.  
**Integration:** the C source is compiled separately and called through C linkage declarations.

## `07 / SAVE DATA`

Use `SAVE` to write the current state to `saves/savegame.txt`. Use `LOAD` to restore it. Save files are generated during play and are excluded from version control.

## `08 / PROJECT NOTES`

- This is an educational terminal-game prototype.
- World topology and initial objects are currently initialized in `Game::createWorld()`.
- `data/rooms.txt` and `data/story.txt` are editable reference content; the current engine does not parse them dynamically at startup.
- The save format is a compact demonstration format.
- The gameplay screenshot and transcript are included as supporting assets.

## `09 / DOCUMENTATION`

- [Game design and technical notes](docs/GAME_DESIGN.md)
- [Gameplay guide and sample route](docs/GAMEPLAY_TRANSCRIPT.md)
- [Project report (Word)](docs/The_Lost_Kingdom_Project_Report.docx)
- [Gameplay transcript](screenshots/gameplay_transcript.txt)
- [Contribution guide](docs/CONTRIBUTING.md)

---

<div align="center">

### `10 / QUALITY CHECKS`

The command parser has a CTest test target. Run it after building:

```bash
ctest --test-dir build --output-on-failure
```

A GitHub Actions workflow configures, builds and tests the project on pushes and pull requests.

### `END OF TRANSMISSION`

**THE LOST KINGDOM**  
*Built with C. Structured with C++. Driven by curiosity.*

</div>