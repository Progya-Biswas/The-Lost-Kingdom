#include "Game.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace lostkingdom {

namespace {
int clampValue(int value, int low, int high) {
    return std::max(low, std::min(value, high));
}
}

Item::Item(std::string name, std::string description)
    : name_(std::move(name)), description_(std::move(description)) {}
std::string Item::name() const { return name_; }
std::string Item::category() const { return "Item"; }
std::string Item::use() const { return "Nothing happens."; }
void Item::describe(std::ostream& out) const { out << name_ << " - " << description_; }
bool Item::operator==(const Item& other) const { return name_ == other.name_; }

Treasure::Treasure(std::string n, std::string d) : Item(std::move(n), std::move(d)) {}
std::string Treasure::category() const { return "Treasure"; }
std::string Treasure::use() const { return "It is valuable, but has no direct use."; }

Tool::Tool(std::string n, std::string d) : Item(std::move(n), std::move(d)) {}
std::string Tool::category() const { return "Tool"; }
std::string Tool::use() const { return "This tool may help with a challenge."; }

Character::Character(std::string name) : name_(std::move(name)) {}
std::string Character::name() const { return name_; }
Guide::Guide() : Character("Mira the Guide") {}
std::string Guide::interact() const {
    return "The three seals are hidden in the library, garden and watchtower. Look carefully.";
}
Guardian::Guardian() : Character("Stone Guardian") {}
std::string Guardian::interact() const {
    return "The Stone Guardian blocks the northern arch. A LUMINOUS KEY may open its seal.";
}

Room::Room(int id, std::string name, std::string description)
    : id_(id), name_(std::move(name)), description_(std::move(description)) {}
int Room::id() const { return id_; }
std::string Room::name() const { return name_; }
void Room::connect(const std::string& direction, int destination) { exits_[direction] = destination; }
bool Room::hasExit(const std::string& direction) const { return exits_.count(direction) > 0; }
int Room::destination(const std::string& direction) const {
    auto it = exits_.find(direction);
    if (it == exits_.end()) throw std::out_of_range("There is no exit in that direction.");
    return it->second;
}
void Room::addItem(std::shared_ptr<Item> item) { items_.push_back(std::move(item)); }
std::shared_ptr<Item> Room::takeItem(const std::string& name) {
    auto it = std::find_if(items_.begin(), items_.end(), [&](const auto& item) {
        return item->name() == name;
    });
    if (it == items_.end()) return nullptr;
    auto result = *it;
    items_.erase(it);
    return result;
}
void Room::addCharacter(std::shared_ptr<Character> character) { characters_.push_back(std::move(character)); }
const std::vector<std::shared_ptr<Item>>& Room::items() const { return items_; }
void Room::describe(std::ostream& out) const {
    out << "\n[" << id_ << "] " << name_ << "\n" << description_ << "\nExits: ";
    if (exits_.empty()) out << "none";
    for (const auto& exit : exits_) out << exit.first << " ";
    out << "\nObjects: ";
    if (items_.empty()) out << "none";
    for (const auto& item : items_) out << item->name() << " (" << item->category() << ")  ";
    if (!characters_.empty()) {
        out << "\nCharacters: ";
        for (const auto& character : characters_) out << character->name() << "  ";
    }
    out << "\n";
}

Player::Player(std::string name)
    : name_(std::move(name)), health_(100), score_(0), roomId_(1) {}
int Player::roomId() const { return roomId_; }
void Player::setRoomId(int id) { roomId_ = id; }
int Player::health() const { return health_; }
int Player::score() const { return score_; }
void Player::addScore(int points) { score_ += points; }
bool Player::pick(std::shared_ptr<Item> item) {
    if (!item || inventory_.size() >= INVENTORY_LIMIT) return false;
    inventory_.push_back(std::move(item));
    return true;
}
std::shared_ptr<Item> Player::drop(const std::string& name) {
    auto it = std::find_if(inventory_.begin(), inventory_.end(), [&](const auto& item) {
        return item->name() == name;
    });
    if (it == inventory_.end()) return nullptr;
    auto result = *it;
    inventory_.erase(it);
    return result;
}
bool Player::hasItem(const std::string& name) const {
    return std::any_of(inventory_.begin(), inventory_.end(), [&](const auto& item) {
        return item->name() == name;
    });
}
void Player::showInventory(std::ostream& out) const {
    out << "Inventory (" << inventory_.size() << "/" << INVENTORY_LIMIT << "): ";
    if (inventory_.empty()) out << "empty";
    for (const auto& item : inventory_) out << item->name() << " [" << item->category() << "]  ";
    out << "\n";
}
void Player::showStatus(std::ostream& out) const {
    out << name_ << " | Health: " << health_ << " | Score: " << score_
        << " | Current room: " << roomId_ << "\n";
}
void Player::damage(int amount) { health_ = clampValue(health_ - amount, 0, 100); }
void Player::restore(int amount) { health_ = clampValue(health_ + amount, 0, 100); }
const std::vector<std::shared_ptr<Item>>& Player::inventory() const { return inventory_; }
void Player::clearInventory() { inventory_.clear(); }
void Player::addLoadedItem(std::shared_ptr<Item> item) { if (item && inventory_.size() < INVENTORY_LIMIT) inventory_.push_back(item); }

Game::Game() : player_("Adventurer"), guardianDefeated_(false), puzzleSolved_(false),
               running_(true), won_(false), treasuresFound_(0), puzzlesSolved_(0) {
    createWorld();
}
void Game::createWorld() {
    rooms_.emplace(1, Room(1, "The Arrival Courtyard", "A cracked fountain stands beneath a faded royal crest."));
    rooms_.emplace(2, Room(2, "The Old Library", "Dusty shelves conceal a faintly glowing seal."));
    rooms_.emplace(3, Room(3, "The Greenhouse", "Wild vines cover the glass roof and a silver tool rests here."));
    rooms_.emplace(4, Room(4, "The Sunken Hall", "Water reflects a mural of the kingdom's three ancient seals."));
    rooms_.emplace(5, Room(5, "The Watchtower", "A stone guardian watches the northern arch."));
    rooms_.emplace(6, Room(6, "The Hidden Vault", "A sealed chamber holds the final royal seal."));
    rooms_.at(1).connect("NORTH", 2); rooms_.at(2).connect("SOUTH", 1);
    rooms_.at(1).connect("EAST", 3);  rooms_.at(3).connect("WEST", 1);
    rooms_.at(2).connect("EAST", 4);  rooms_.at(4).connect("WEST", 2);
    rooms_.at(3).connect("SOUTH", 4); rooms_.at(4).connect("NORTH", 3);
    rooms_.at(4).connect("EAST", 5);  rooms_.at(5).connect("WEST", 4);
    rooms_.at(5).connect("NORTH", 6); rooms_.at(6).connect("SOUTH", 5);

    rooms_.at(2).addItem(std::make_shared<Treasure>("ANCIENT SEAL", "A small seal bearing a library emblem."));
    rooms_.at(3).addItem(std::make_shared<Tool>("LUMINOUS KEY", "A key that glows near sealed doors."));
    rooms_.at(4).addItem(std::make_shared<Treasure>("ROYAL COIN", "A rare coin from the old kingdom."));
    rooms_.at(1).addCharacter(std::make_shared<Guide>());
    rooms_.at(5).addCharacter(std::make_shared<Guardian>());
}
void Game::showHelp() const {
    std::cout << "\nCommands: HELP, LOOK, MOVE <direction>, PICK <item>, DROP <item>,\n"
                 "INVENTORY, USE <item>, TALK, ATTACK, STATUS, MAP, SAVE, LOAD, EXIT\n";
}
void Game::move(const std::string& direction) {
    auto& room = rooms_.at(player_.roomId());
    if (!room.hasExit(direction)) {
        std::cout << "You cannot move " << direction << ". A wall blocks the way.\n";
        return;
    }
    if (player_.roomId() == 5 && direction == "NORTH" && !guardianDefeated_) {
        std::cout << "The Stone Guardian blocks the arch. Use the LUMINOUS KEY first.\n";
        return;
    }
    player_.setRoomId(room.destination(direction));
    rooms_.at(player_.roomId()).describe(std::cout);
}
void Game::pick(const std::string& name) {
    auto item = rooms_.at(player_.roomId()).takeItem(name);
    if (!item) { std::cout << "That object is not in this location.\n"; return; }
    if (!player_.pick(item)) {
        rooms_.at(player_.roomId()).addItem(item);
        std::cout << "Your inventory is full (maximum 6). The object remains here.\n";
        return;
    }
    if (item->category() == "Treasure") {
        ++treasuresFound_;
        player_.addScore(calculate_score(1, 0, 0));
    }
    std::cout << "Collected: " << item->name() << "\n";
}
void Game::drop(const std::string& name) {
    auto item = player_.drop(name);
    if (!item) { std::cout << "You do not have that object.\n"; return; }
    rooms_.at(player_.roomId()).addItem(item);
    std::cout << "Dropped: " << item->name() << "\n";
}
void Game::use(const std::string& name) {
    if (!player_.hasItem(name)) { std::cout << "You are not carrying that item.\n"; return; }
    if (name == "LUMINOUS KEY" && player_.roomId() == 5 && !guardianDefeated_) {
        guardianDefeated_ = true;
        ++puzzlesSolved_;
        player_.addScore(calculate_score(0, 1, 0));
        std::cout << "The key shines brightly. The guardian's seal dissolves; the northern arch opens!\n";
    } else if (name == "ANCIENT SEAL" && player_.roomId() == 6) {
        if (!puzzleSolved_) {
            puzzleSolved_ = true;
            ++puzzlesSolved_;
            player_.addScore(calculate_score(0, 1, 0));
            std::cout << "The seal fits the vault altar. The Lost Kingdom's mystery is solved!\n";
        } else std::cout << "The vault puzzle is already solved.\n";
    } else {
        std::cout << "You try the item. " << name << ": ";
        auto it = std::find_if(player_.inventory().begin(), player_.inventory().end(),
            [&](const auto& item) { return item->name() == name; });
        if (it != player_.inventory().end()) std::cout << (*it)->use() << "\n";
    }
}
void Game::save() const {
    std::ofstream out("saves/savegame.txt");
    if (!out) throw std::runtime_error("Could not open save file for writing.");
    out << player_.roomId() << "\n" << player_.health() << "\n" << player_.score() << "\n"
        << guardianDefeated_ << "\n" << puzzleSolved_ << "\n" << treasuresFound_ << "\n" << puzzlesSolved_ << "\n";
    for (const auto& item : player_.inventory()) out << item->category() << "|" << item->name() << "\n";
    std::cout << "Game saved to saves/savegame.txt\n";
}
void Game::load() {
    std::ifstream in("saves/savegame.txt");
    if (!in) { std::cout << "No save file found. Play a little and use SAVE first.\n"; return; }
    int room, health, score, treasureCount, puzzleCount;
    bool guardian, puzzle;
    if (!(in >> room >> health >> score >> guardian >> puzzle >> treasureCount >> puzzleCount))
        throw std::runtime_error("Save file is incomplete or damaged.");
    std::string line;
    std::getline(in, line);
    player_.clearInventory();
    while (std::getline(in, line)) {
        auto split = line.find('|');
        if (split == std::string::npos) continue;
        std::string type = line.substr(0, split), name = line.substr(split + 1);
        if (type == "Treasure") player_.addLoadedItem(std::make_shared<Treasure>(name, "Restored from save."));
        else player_.addLoadedItem(std::make_shared<Tool>(name, "Restored from save."));
    }
    player_.setRoomId(rooms_.count(room) ? room : 1);
    player_.restore(health - player_.health());
    player_.addScore(score - player_.score());
    guardianDefeated_ = guardian; puzzleSolved_ = puzzle;
    treasuresFound_ = treasureCount; puzzlesSolved_ = puzzleCount;
    std::cout << "Saved game loaded.\n";
}
void Game::showMap() const {
    std::cout << "\nMAP: [1] Courtyard -- [2] Library -- [4] Hall -- [5] Watchtower -- [6] Vault\n"
                 "                         [3] Greenhouse\n";
}
void Game::checkEnding() {
    if (player_.health() <= 0) {
        std::cout << "\nYour strength fades. The kingdom remains lost. GAME OVER.\n";
        running_ = false;
    } else if (player_.roomId() == 6 && puzzleSolved_) {
        std::cout << "\nVICTORY! You restored the final seal and uncovered the kingdom's secret.\n";
        running_ = false; won_ = true;
    }
}
void Game::run() {
    std::cout << "=====================================\n"
                 "          THE LOST KINGDOM\n"
                 "=====================================\n"
                 "Explore the kingdom, collect its seal and solve the vault puzzle.\n"
                 "Type HELP to see commands. Type EXIT to leave.\n";
    rooms_.at(1).describe(std::cout);
    std::string line;
    while (running_) {
        std::cout << "\n> ";
        if (!std::getline(std::cin, line)) break;
        ParsedCommand command{};
        if (!parse_command(line.c_str(), &command)) continue;
        try {
            switch (command.type) {
                case CMD_HELP: showHelp(); break;
                case CMD_LOOK: rooms_.at(player_.roomId()).describe(std::cout); break;
                case CMD_MOVE: move(command.argument); break;
                case CMD_PICK: pick(command.argument); break;
                case CMD_DROP: drop(command.argument); break;
                case CMD_INVENTORY: player_.showInventory(std::cout); break;
                case CMD_USE: use(command.argument); break;
                case CMD_STATUS: player_.showStatus(std::cout); break;
                case CMD_SAVE: save(); break;
                case CMD_LOAD: load(); break;
                case CMD_MAP: showMap(); break;
                case CMD_TALK:
                    if (player_.roomId() == 1) std::cout << Guide().interact() << "\n";
                    else if (player_.roomId() == 5) std::cout << Guardian().interact() << "\n";
                    else std::cout << "There is nobody here to talk to.\n";
                    break;
                case CMD_ATTACK:
                    if (player_.roomId() == 5 && !guardianDefeated_) {
                        player_.damage(25);
                        std::cout << "The guardian repels your attack. Health reduced by 25.\n";
                    } else std::cout << "There is no hostile character to attack here.\n";
                    break;
                case CMD_EXIT: running_ = false; std::cout << "Thanks for playing!\n"; break;
                default: std::cout << "Unknown command. Type HELP for instructions.\n"; break;
            }
            checkEnding();
        } catch (const std::exception& ex) {
            std::cout << "Error: " << ex.what() << "\n";
        }
    }
}
} // namespace lostkingdom