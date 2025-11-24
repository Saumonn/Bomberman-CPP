#ifndef BOMBERMAN_H
#define BOMBERMAN_H

#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>

using namespace std;


class Element {
public:
    virtual ~Element() = default;

    virtual char getSymbol() const = 0;
    virtual bool isWalkable() const { return false; }
    virtual bool isDestructible() const { return false; }
    virtual bool isPlayer() const { return false; }
    virtual bool isBonus() const { return false; }
};

// Case vide
class Empty : public Element {
public:
    char getSymbol() const override { return ' '; }
    bool isWalkable() const override { return true; }
};

// Mur destructible '%'
class DestructibleWall : public Element {
public:
    char getSymbol() const override { return '%'; }
    bool isWalkable() const override { return false; }
    bool isDestructible() const override { return true; }
};

// Mur indestructible '#'
class IndestructibleWall : public Element {
public:
    char getSymbol() const override { return '#'; }
    bool isWalkable() const override { return false; }
    bool isDestructible() const override { return false; }
};

// Bombe 'B'
class Bomb : public Element {
private:
    int m_timer;
    int m_power;
public:
    Bomb(int timer, int power) : m_timer(timer), m_power(power) {}

    char getSymbol() const override { return 'B'; }
    bool isWalkable() const override { return false; }
    bool isDestructible() const override { return true; }
};

// Bonus 'b'
class Bonus : public Element {
private:
    int m_powerIncrease;
public:
    Bonus(int powerIncrease) : m_powerIncrease(powerIncrease) {}

    char getSymbol() const override { return 'b'; }
    bool isWalkable() const override { return true; }
    bool isDestructible() const override { return true; }
    bool isBonus() const override { return true; }

    int getPowerIncrease() const { return m_powerIncrease; }
};

// Feu '*'
class Fire : public Element {
public:
    char getSymbol() const override { return '*'; }
    bool isWalkable() const override { return false; }
    bool isDestructible() const override { return true; }
};

// Joueur '1'..'4'
class Player : public Element {
private:
    int  m_id;
    int  m_x;
    int  m_y;
    int  m_power;
    bool m_alive;
    bool m_hasActiveBomb;

public:
    Player(int id, int x, int y, int power = 1)
        : m_id(id),
          m_x(x),
          m_y(y),
          m_power(power),
          m_alive(true),
          m_hasActiveBomb(false)
    {}

    char getSymbol() const override { return '0' + m_id; }
    bool isWalkable() const override { return false; }
    bool isDestructible() const override { return true; }
    bool isPlayer() const override { return true; }

    int  getId()    const { return m_id; }
    int  getX()     const { return m_x; }
    int  getY()     const { return m_y; }
    int  getPower() const { return m_power; }
    bool isAlive()  const { return m_alive; }

    void setPosition(int x, int y) { m_x = x; m_y = y; }
    void setPower(int power)       { m_power = power; }
    void kill()                    { m_alive = false; }

    bool hasActiveBomb()     const { return m_hasActiveBomb; }
    void setHasActiveBomb(bool v)  { m_hasActiveBomb = v; }
};

class Bomberman {
private:
    int m_width;
    int m_height;
    vector<vector<Element*>> m_grid;

    Player* m_players[4];

    struct BombData {
        int x;
        int y;
        int timer;
        int power;
        int ownerId;
        bool active;
    };

    struct FireData {
        int x;
        int y;
        int timer;
        bool spawnBonus;
    };

    vector<BombData> m_bombs;
    vector<FireData> m_fires;

public:
    Bomberman();
    ~Bomberman();

    void operator<<(vector<string> level);

    int getWidth()  const { return m_width; }
    int getHeight() const { return m_height; }

    Element* getCell(int x, int y) const { return m_grid[y][x]; }

    bool movePlayer(int playerId, int dx, int dy);

    bool placeBomb(int playerId);
    bool hasBombAt(int x, int y) const;

    void update();

    // Accès aux joueurs (pour la view)
    Player* getPlayer(int id) const;
    bool    isPlayerAlive(int id) const;
    int     countAlivePlayers() const;

private:

    void clearGrid();
    void resetPlayers();
    Element* createElementFromChar(char c, int x, int y);

    bool isInside(int x, int y) const {
        return x >= 0 && x < m_width && y >= 0 && y < m_height;
    }

    void updateBombs();
    void updateFires();

    void explodeBomb(int index);
    void createFireAt(int x, int y);
};

#endif // BOMBERMAN_H
