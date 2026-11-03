<div align="center">

# ✦ THE LOST KINGDOM ✦
### A small adventure through a kingdom that time forgot.

**C11 · C++17 · raylib · CMake**

![C11](https://img.shields.io/badge/C-11-7B5BC7?style=flat-square&logo=c&logoColor=white)
![C++17](https://img.shields.io/badge/C++-17-9A78D6?style=flat-square&logo=cplusplus&logoColor=white)
![raylib](https://img.shields.io/badge/graphics-raylib_6.2-B89BE8?style=flat-square)
![CMake](https://img.shields.io/badge/build-CMake-6E55A4?style=flat-square&logo=cmake&logoColor=white)

◇ ───────────────────────────────────────── ◇

*Explore quiet halls. Find forgotten objects. Bring the kingdom back to life.*

</div>

---

## Welcome to Eldoria

The Lost Kingdom is a 2D, top-down fantasy adventure. You arrive in a forgotten kingdom with a simple task: find the objects that have been hidden across its rooms, get past the Stone Guardian, and restore the seal in the Hidden Vault.

The game is drawn in a dark purple and lavender palette. Its rooms, doors, objects, characters and interface are made from geometric shapes using **raylib**. You can walk around the world, interact with people and objects, keep an eye on your objectives, and open an inventory or map whenever you need one.

The project also keeps its original terminal version. That makes it possible to explore the same C command parser and C++ game model without the graphical window.

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
        ┌──────────────────────┐
        │     OLD LIBRARY      │
        │     ANCIENT SEAL     │
        └──────────┬───────────┘
                   │
┌─────────────────┐│   ┌──────────────────┐
│ ARRIVAL         ├┘   │ GREENHOUSE       │
│ COURTYARD       │────│ LUMINOUS KEY     │
│ MIRA THE GUIDE  │    └────────┬─────────┘
└─────────────────┘             │
                                ▼
                       ┌──────────────────┐
                       │   SUNKEN HALL    │
                       └────────┬─────────┘
                                │
                                ▼
                       ┌──────────────────┐
                       │   WATCHTOWER     │
                       │ STONE GUARDIAN   │
                       └────────┬─────────┘
                                │
                         KEY OPENS THE WAY
                                │
                                ▼
                       ┌──────────────────┐
                       │   HIDDEN VAULT   │
                       │   FINAL ALTAR    │
                       └──────────────────┘
```

## How to play

The graphical game opens with a title screen. Press **Enter** to begin.

| Key | What it does |
|---|---|
| **W A S D** or **arrow keys** | Walk around the room |
| **E** | Talk to someone or interact with a nearby object |
| **I** | Open or close your inventory |
| **M** | Open or close the world map |
| **H** | Show the help message |
| **/** | Open the typed command console |
| **Enter** | Run a command in the console |
| **Backspace** | Edit a command |
| **F5 / F9** | Save / load your progress |

### Your first playthrough

1. In the Arrival Courtyard, walk close to Mira and press **E**.
2. Take the northern doorway to the Old Library. Walk close to the Ancient Seal and press **E**.
3. Return to the courtyard and take the eastern doorway to the Greenhouse.
4. Collect the Luminous Key with **E**.
5. Go south to the Sunken Hall, then east to the Watchtower.
6. Walk up to the Stone Guardian and press **E**. The key opens the northern arch.
7. Enter the Hidden Vault. Approach the altar and press **E** while carrying the Ancient Seal.

The optional Royal Coin in the Sunken Hall adds to your score.

## What went into building it?

This project brings together two languages and a graphics library.

### C11 — the command utilities

C handles the small, reusable command-processing layer. The parser takes text such as `MOVE NORTH`, identifies the command, and places the result into a structure that the C++ side can use.

A few C concepts appear directly in this module:

- **Enumeration (`enum`)** — `CommandType` gives each command a readable identifier such as `CMD_MOVE` or `CMD_PICK`.
- **Structure (`struct`)** — `ParsedCommand` keeps the command type and its argument together.
- **Pointers and `const`** — the parser receives the input string and an output pointer, while protecting the original input from modification.
- **Character arrays and strings** — a bounded buffer, `strtok`, `toupper` and `strcmp` split and normalise the user's command.
- **Functions and return values** — `calculate_score()` applies the score formula and `parse_command()` reports whether parsing succeeded.
- **Dynamic memory** — `duplicate_text()` demonstrates `malloc`, `memcpy` and caller-owned memory that must eventually be released with `free`.

### C++17 — the game model

C++ is used for the objects and behaviour that make up the kingdom. The original game model includes classes for `Item`, `Treasure`, `Tool`, `Character`, `Guide`, `Guardian`, `Room`, `Player` and `Game`.

The main OOP ideas are not just vocabulary in this project; they have a place in the code:

- **Encapsulation** — classes keep their state private and expose methods for controlled changes.
- **Inheritance** — `Treasure` and `Tool` extend `Item`; `Guide` and `Guardian` extend the abstract `Character` class.
- **Virtual functions and overriding** — derived item classes provide their own `category()` and `use()` behaviour.
- **Abstract classes** — `Character::interact()` is a pure virtual function that defines an interface for characters.
- **Runtime polymorphism** — rooms and inventories hold items through `std::shared_ptr<Item>`, allowing virtual item behaviour to be selected from the actual object type.
- **Operator overloading** — `Item::operator==` compares items by name.
- **STL and lambdas** — `std::map`, `std::vector`, `std::find_if` and lambda expressions manage rooms and inventory.
- **File handling and exceptions** — streams save and restore progress, while exceptions protect the game from invalid operations.

### raylib — bringing the rooms to life

The graphical controller is written in C++. raylib provides the desktop window, keyboard input and drawing functions. The scene is built with geometric primitives: rectangles form the floor tiles and interface panels; circles and polygons create the decorative objects, relics and characters.

The game loop updates movement using frame delta time, handles keyboard input, and redraws the scene. The purple identity runs through the room palettes, panels, highlights and title screen, with lavender used for readable text and important details.

### How the two languages work together

The typed command console sends its text to the C function `parse_command()`. C returns a `ParsedCommand` containing an enum value and argument. The C++ controller reads that result and calls the matching game action.

The shared C header uses `extern "C"` when it is included by C++ code. This preserves C language linkage and lets the C++ program call the C functions. CMake compiles the C source as C11 and the game code as C++17 before linking the targets.

## Build and run on Windows

### What you need

- Windows 10 or 11
- Visual Studio 2022 with **Desktop development with C++**
- CMake (included with the Visual Studio C++ workload)
- Internet access for the first build, so CMake can fetch raylib 6.2

### 1. Download the source

Open this repository, select **Code → Download ZIP**, and extract the downloaded folder.

### 2. Open the right terminal

Open **Developer PowerShell for Visual Studio 2022** from the Start menu. Change directory to the extracted project folder. For example:

```powershell
cd "$HOME\Downloads\The-Lost-Kingdom-main"
```

The extracted folder may have a slightly different name on your computer.

### 3. Configure and compile

```powershell
cmake -S . -B build
cmake --build build --config Release
```

CMake downloads the pinned raylib 6.2 dependency during the first configuration.

### 4. Open the game

```powershell
.\build\Release\lost_kingdom.exe
```

The graphical window should open. Press **Enter** on the title screen.

### Optional: play the terminal edition

The original command-line edition is kept as a separate target:

```powershell
.\build\Release\lost_kingdom_cli.exe
```

## A look at the codebase

```text
The-Lost-Kingdom/
├── .github/workflows/   # Automated build and test workflow
├── c_modules/           # C command parser and utilities
├── cpp_modules/         # C++ object-oriented terminal game model
├── visual/              # raylib graphical controller and drawing
├── include/              # Shared headers and class declarations
├── tests/                # C command-parser tests
├── data/                 # Room and story reference text
├── docs/                 # Design notes and gameplay guide
├── screenshots/          # Gameplay reference assets
├── saves/                # Local save-file location
├── main.cpp              # Graphical application entry point
├── cli_main.cpp          # Terminal application entry point
└── CMakeLists.txt        # C11/C++17, raylib and test build setup
```

## Build checks

The GitHub Actions workflow builds the graphical and terminal targets on Windows and Linux, then runs the C command-parser tests with CTest.

To run the parser tests locally after building:

```bash
ctest --test-dir build --output-on-failure
```

## A few honest notes

The first graphical edition uses art drawn from raylib shapes rather than external sprite sheets. The room layouts and object positions are currently set in the C++ source; the text files in `data/` are reference material and are not loaded dynamically. Save/load is a lightweight project feature, not a production save system.

There is room to grow this world: animated sprite sheets, sound, more puzzles, additional rooms and richer character interactions would all be natural next steps.

## Project documentation

- [Graphical game guide](docs/GRAPHICAL_GAME_GUIDE.md)
- [Game design and technical notes](docs/GAME_DESIGN.md)
- [Terminal edition sample commands](docs/GAMEPLAY_TRANSCRIPT.md)
- [C and C++ project report (Word)](docs/The_Lost_Kingdom_Project_Report.docx)

---

<div align="center">

◇ **THE LOST KINGDOM** ◇

*Made with C, C++ and a little imagination.*

**A forgotten world. A violet glow. A story waiting to be finished.**

</div>