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
- Game name, score, and health are located at the top of the screen. 
- The screen will be composed of a 12 by 8 grid of tiles to represent rooms. Doors, decorations and items/npcs and the player.  
- Quest goals.  Satchel and Story on the right side of the screen. 
- Keys for moving around and interacting at the bottom of the screen. 
- Will have a start screen and win screen. 
- Will have an inventory and map screen.  
- Can type out commands (maybe).  

Using raylibs basic functions I will draw everything out. Different colors and decoration for each room to tell them apart. 

## 4. Controls
Will use W, A, S, D, and Arrow keys to move around. E will allow you to interact with objects and NPCs. I opens your inventory.  M opens the map.  H shows you controls.  / allows you to type out commands.  Enter submits your typed command.  F5 saves and F9 loads. 

## 5. C/C++ architecture

### C11 module
- `include/c_utils.h`: `CommandType` enum, `ParsedCommand` struct, and C function declarations.
- `c_modules/c_utils.c`: command parsing, uppercase normalization, score calculation, and dynamic string duplication.
- The C parser validates pointers and uses bounded copying before tokenization.

### C++17 modules
- `include/Game.hpp` and `cpp_modules/Game.cpp`: original object-oriented game model and terminal engine.
- `visual/VisualGame.hpp` and `visual/VisualGame.cpp`: graphical application controller and raylib drawing/event loop.
- `main.cpp`: graphical entry point.
- `cli_main.cpp`: optional terminal entry point.

### C/C++ integration
The graphical command console and GUI commands invoke the C parser. The C parser will return a ParsedCommand, which our C++ Controller will then use to run the correct command. The header has been written using extern "C" in C++ to maintain the linkage of the C language. 

Using CMake, we are able to build our project as C11 for the C module. Build out games as C++17 and get our pinned raylib dependency version 6.2. Then link to the executable files.  

## 6. OOP and STL concepts
- Encapsulation: Player, Room, Item, and Game keep state private.
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
F5 writes the current room, score, quest item flags, and Guardian state to `saves/graphical_save.txt`. F9 restores those values. The file format is intentionally simple for a teaching project.

## 9. Build and test
See the root README and docs/GRAPHICAL_GAME_GUIDE.md. CTest runs the C command parser tests. GitHub Actions builds both the graphical and terminal targets and executes those tests.

## 10. Limitations and possible extensions
Room topologies and placement of objects in rooms are hard-coded at this time. All files located in data/ are used for reference and do not represent rooms that will be played in the game. Currently, we are using art from raylib to create our graphics. We can implement sprites, sound, animated sprites for the player, and more tests on C++ gameplay in the future. 
