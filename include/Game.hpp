#pragma once

#include "c_utils.h"
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace lostkingdom {

class Item {
public:
    Item(std::string name, std::string description);
    virtual ~Item() = default;
    std::string name() const;
    virtual std::string category() const;
    virtual std::string use() const;
    virtual void describe(std::ostream& out) const;
    bool operator==(const Item& other) const;
private:
    std::string name_;
    std::string description_;
};

class Treasure : public Item {
public:
    Treasure(std::string name, std::string description);
    std::string category() const override;
    std::string use() const override;
};

class Tool : public Item {
public:
    Tool(std::string name, std::string description);
    std::string category() const override;
    std::string use() const override;
};

class Character {
public:
    explicit Character(std::string name);
    virtual ~Character() = default;
    std::string name() const;
    virtual std::string interact() const = 0;
private:
    std::string name_;
};

class Guide : public Character {
public:
    Guide();
    std::string interact() const override;
};

class Guardian : public Character {
public:
    Guardian();
    std::string interact() const override;
};

class Room {
public:
    Room(int id, std::string name, std::string description);
    int id() const;
    std::string name() const;
    void connect(const std::string& direction, int destination);
    bool hasExit(const std::string& direction) const;
    int destination(const std::string& direction) const;
    void addItem(std::shared_ptr<Item> item);
    std::shared_ptr<Item> takeItem(const std::string& name);
    void addCharacter(std::shared_ptr<Character> character);
    const std::vector<std::shared_ptr<Item>>& items() const;
    void describe(std::ostream& out) const;
private:
    int id_;
    std::string name_;
    std::string description_;
    std::map<std::string, int> exits_;
    std::vector<std::shared_ptr<Item>> items_;
    std::vector<std::shared_ptr<Character>> characters_;
};

class Player {
public:
    explicit Player(std::string name = "Explorer");
    int roomId() const;
    void setRoomId(int id);
    int health() const;
    int score() const;
    void addScore(int points);
    void damage(int points);
    void restore(int points);
    bool pick(std::shared_ptr<Item> item);
    std::shared_ptr<Item> drop(const std::string& name);
    bool hasItem(const std::string& name) const;
    const std::vector<std::shared_ptr<Item>>& inventory() const;
    void clearInventory();
    void addLoadedItem(std::shared_ptr<Item> item);
    void showInventory(std::ostream& out) const;
    void showStatus(std::ostream& out) const;
private:
    std::string name_;
    int health_;
    int score_;
    int roomId_;
    std::vector<std::shared_ptr<Item>> inventory_;
    static constexpr std::size_t INVENTORY_LIMIT = 6;
};

class Game {
public:
    Game();
    void run();
private:
    std::map<int, Room> rooms_;
    Player player_;
    bool guardianDefeated_ = false;
    bool puzzleSolved_ = false;
    bool running_ = true;
    bool won_ = false;
    int treasuresFound_ = 0;
    int puzzlesSolved_ = 0;
    void createWorld();
    void showHelp() const;
    void move(const std::string& direction);
    void pick(const std::string& itemName);
    void drop(const std::string& itemName);
    void use(const std::string& itemName);
    void save() const;
    void load();
    void showMap() const;
    void checkEnding();
};

} // namespace lostkingdom