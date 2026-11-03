# The Lost Kingdom — Game Design & Technical Notes

## 1. Overview
The Lost Kingdom is a small 2D top-down fantasy adventure built with C11 and C++17. The graphical edition uses raylib 6.2 to display rooms, characters, items, doors, the quest tracker and inventory. The original terminal edition is kept as an optional executable.

## 2. World and progression

| ID | Location | Role |
|---:|---|---|
| 1 | Arrival Courtyard | Starting area; meet Mira the Guide |
| 2 | Old Library | Find the Ancient Seal |
| 3 | Greenhouse | Find the Luminous Key |
| 4 | Sunken Hall | Optional Royal Coin and route between regions |
| 5 | Watchtower | Use the Luminous Key to open the Guardian barrier |
| 6 | Hidden Vault | Use the Ancient Seal to complete the quest |

The intended route is Courtyard → Library → Courtyard → Greenhouse → Sunken Hall → Watchtower → Hidden Vault.

## 3. Graphical interface
- Top bar: game title, score and health.
- Main viewport: 12 × 8 room tile grid, doors, room decorations, objects, NPCs and player.
- Right panel: quest objectives, satchel/inventory and story log.
- Bottom strip: movement and action shortcuts.
- Start and victory screens.
- Inventory and world map overlays.
- Optional typed command overlay.

The initial visual style is drawn with raylib primitives. Room color palettes and decorations distinguish the six locations without external sprite assets.

## 4. Controls
WASD or arrow keys move the player. E interacts with nearby objects or characters. I toggles inventory, M toggles the world map, H displays help, slash opens the typed command console, Enter executes a typed command, F5 saves, and F9 loads.

## 5. C/C++ architecture

### C11 module
- `include/c_utils.h`: `CommandType` enum, `ParsedCommand` struct and C function declarations.
- `c_modules/c_utils.c`: command parsing, uppercase normalization, score calculation and dynamic string duplication.
- The C parser validates pointers and uses bounded copying before tokenization.

### C++17 modules
- `include/Game.hpp` and `cpp_modules/Game.cpp`: original object-oriented game model and terminal engine.
- `visual/VisualGame.hpp` and `visual/VisualGame.cpp`: graphical application controller and raylib drawing/event loop.
- `main.cpp`: graphical entry point.
- `cli_main.cpp`: optional terminal entry point.

### C/C++ integration
The graphical command console and GUI actions call the C parser. The C parser returns a `ParsedCommand`, and the C++ controller dispatches the corresponding command. The header uses `extern "C"` under C++ to preserve C language linkage.

CMake compiles the C module as C11, the game modules as C++17, fetches the pinned raylib 6.2 dependency, and links the executables.

## 6. OOP and STL concepts
- Encapsulation: Player, Room, Item and Game keep state private.
- Inheritance: Treasure and Tool derive from Item; Guide and Guardian derive from Character.
- Virtual functions: Item category/use calls dispatch by derived type through base-class pointers.
- Abstract class: Character declares a pure virtual interact method.
- Polymorphism: room and inventory collections store `shared_ptr<Item>` and call virtual item methods.
- Operator overloading: Item implements equality by comparing names.
- STL: map, vector, shared_ptr, find_if and lambdas.
- Namespaces: `lostkingdom` scopes the terminal engine classes.

## 7. Graphics programming concepts
The graphical layer demonstrates:
- Window initialization and shutdown.
- A real-time game loop with delta-time movement.
- Keyboard polling and event handling.
- Drawing primitives, colors, rounded panels and text.
- Room tile grid and collision/boundary handling.
- HUD, overlays, inventory and quest tracking.
- State transitions between title, play and victory screens.

## 8. Save/load
F5 writes the current room, score, quest item flags and Guardian state to `saves/graphical_save.txt`. F9 restores those values. The file format is intentionally simple for a teaching project.

## 9. Build and test
See the root README and docs/GRAPHICAL_GAME_GUIDE.md. CTest runs the C command parser tests. GitHub Actions builds both the graphical and terminal targets and executes those tests.

## 10. Limitations and possible extensions
Room topology and visual placement are currently defined in the source code. Files under data/ are reference material rather than runtime-loaded room definitions. Art is currently procedural, using raylib primitives; sprite sheets, sound, animated character sprites, additional levels and more comprehensive C++ gameplay tests are possible future improvements.
