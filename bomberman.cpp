#include "bomberman.h"

#include <cstdlib>
#include <ctime>

using namespace std;

Bomberman::Bomberman() : m_width(0), m_height(0), m_grid() {
    resetPlayers();
    srand(static_cast<unsigned>(time(nullptr)));
}

Bomberman::~Bomberman() { clearGrid(); m_bombs.clear(); m_fires.clear(); }

void Bomberman::clearGrid() {
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            delete m_grid[y][x];
        }
    }
    m_grid.clear();
    m_width  = 0;
    m_height = 0;
}

void Bomberman::resetPlayers() {
    for (int i = 0; i < 4; ++i) {
        m_players[i] = nullptr;
    }
}

Element* Bomberman::createElementFromChar(char c, int x, int y) {
    switch (c) {
        case '#': return new IndestructibleWall();
        case '%': return new DestructibleWall();
        case 'B': return new Bomb(3, 1);
        case 'b': return new Bonus(1);
        case '*': return new Fire();
        case '1':
        case '2':
        case '3':
        case '4': {
            int id = c - '0';
            Player* p = new Player(id, x, y);
            m_players[id - 1] = p;
            return p;
        }
        default:
            return new Empty();
    }
}

void Bomberman::operator<<(vector<string> level) {
    clearGrid();
    resetPlayers();
    m_bombs.clear();
    m_fires.clear();

    m_height = static_cast<int>(level.size());
    if (m_height == 0) {
        m_width = 0;
        return;
    }

    m_width = static_cast<int>(level[0].size());
    m_grid.resize(m_height);

    for (int y = 0; y < m_height; ++y) {
        m_grid[y].resize(m_width);
        for (int x = 0; x < m_width; ++x) {
            char c = (x < static_cast<int>(level[y].size())) ? level[y][x] : ' ';
            m_grid[y][x] = createElementFromChar(c, x, y);
        }
    }
}

bool Bomberman::movePlayer(int playerId, int dx, int dy) {
    if (playerId < 1 || playerId > 4) {
        return false;
    }

    Player* p = m_players[playerId - 1];
    if (p == nullptr || !p->isAlive()) {
        return false;
    }

    int px = p->getX();
    int py = p->getY();

    int nx = px + dx;
    int ny = py + dy;

    if (!isInside(nx, ny)) {
        return false;
    }

    if (hasBombAt(nx, ny)) {
        return false;
    }

    Element* target = m_grid[ny][nx];

    if (!target->isWalkable()) {
        return false;
    }

    if (target->isBonus()) {
        Bonus* bonus = dynamic_cast<Bonus*>(target);
        int inc = 1;
        if (bonus) {
            inc = bonus->getPowerIncrease();
        }
        p->setPower(p->getPower() + inc);
    }

    Element* oldTarget = target;

    m_grid[py][px] = new Empty();
    m_grid[ny][nx] = p;
    p->setPosition(nx, ny);

    delete oldTarget;

    return true;
}

bool Bomberman::placeBomb(int playerId) {
    if (playerId < 1 || playerId > 4) {
        return false;
    }

    Player* p = m_players[playerId - 1];
    if (p == nullptr || !p->isAlive()) {
        return false;
    }

    if (p->hasActiveBomb()) {
        return false;
    }

    int x = p->getX();
    int y = p->getY();

    if (hasBombAt(x, y)) {
        return false;
    }

    BombData data;
    data.x       = x;
    data.y       = y;
    data.timer   = 20;
    data.power   = p->getPower();
    data.ownerId = playerId;
    data.active  = true;

    m_bombs.push_back(data);

    p->setHasActiveBomb(true);

    return true;
}

bool Bomberman::hasBombAt(int x, int y) const {
    for (const auto& b : m_bombs) {
        if (b.active && b.x == x && b.y == y) {
            return true;
        }
    }
    return false;
}

void Bomberman::update() {
    updateBombs();
    updateFires();
}

void Bomberman::updateBombs() {
    vector<int> toExplode;

    for (int i = 0; i < static_cast<int>(m_bombs.size()); ++i) {
        BombData& b = m_bombs[i];
        if (!b.active) continue;

        b.timer--;
        if (b.timer <= 0) {
            toExplode.push_back(i);
        }
    }

    for (int idx : toExplode) {
        if (idx >= 0 && idx < static_cast<int>(m_bombs.size()) && m_bombs[idx].active) {
            explodeBomb(idx);
        }
    }

    for (int i = static_cast<int>(m_bombs.size()) - 1; i >= 0; --i) {
        if (!m_bombs[i].active) {
            m_bombs.erase(m_bombs.begin() + i);
        }
    }
}

void Bomberman::updateFires() {
    for (int i = static_cast<int>(m_fires.size()) - 1; i >= 0; --i) {
        FireData& f = m_fires[i];
        f.timer--;

        if (f.timer <= 0) {
            int x = f.x;
            int y = f.y;

            Element* e = m_grid[y][x];

            if (e->getSymbol() == '*') {
                delete e;

                if (f.spawnBonus) {
                    m_grid[y][x] = new Bonus(1);
                } else {
                    m_grid[y][x] = new Empty();
                }
            }

            m_fires.erase(m_fires.begin() + i);
        }
    }
}

void Bomberman::createFireAt(int x, int y) {
    Element* e = m_grid[y][x];

    bool willSpawnBonus = false;

    if (e->isPlayer()) {
        Player* p = dynamic_cast<Player*>(e);
        if (p) {
            p->kill();
        }
    } else {
        if (e->getSymbol() == '%') {
            int r = std::rand() % 100;
            if (r < 30) {
                willSpawnBonus = true;
            }
        }
        delete e;
    }

    m_grid[y][x] = new Fire();

    FireData f;
    f.x          = x;
    f.y          = y;
    f.timer      = 10;
    f.spawnBonus = willSpawnBonus;

    m_fires.push_back(f);
}

void Bomberman::explodeBomb(int index) {
    if (index < 0 || index >= static_cast<int>(m_bombs.size())) return;

    BombData& b = m_bombs[index];
    if (!b.active) return;

    b.active = false;

    int ownerId = b.ownerId;
    if (ownerId >= 1 && ownerId <= 4) {
        Player* owner = m_players[ownerId - 1];
        if (owner != nullptr) {
            owner->setHasActiveBomb(false);
        }
    }

    int bx    = b.x;
    int by    = b.y;
    int power = b.power;

    createFireAt(bx, by);

    const int dx[4] = { 1, -1,  0,  0 };
    const int dy[4] = { 0,  0,  1, -1 };

    for (int dir = 0; dir < 4; ++dir) {
        for (int step = 1; step <= power; ++step) {
            int nx = bx + dx[dir] * step;
            int ny = by + dy[dir] * step;

            if (!isInside(nx, ny)) {
                break;
            }

            for (int j = 0; j < static_cast<int>(m_bombs.size()); ++j) {
                BombData& other = m_bombs[j];
                if (other.active && other.x == nx && other.y == ny) {
                    explodeBomb(j);
                }
            }

            Element* e = m_grid[ny][nx];

            if (e->isPlayer()) {
                createFireAt(nx, ny);
                continue;
            }

            bool walkable     = e->isWalkable();
            bool destructible = e->isDestructible();

            if (!walkable && !destructible) {
                break;
            }

            createFireAt(nx, ny);

            if (!walkable && destructible) {
                break;
            }
        }
    }
}

Player* Bomberman::getPlayer(int id) const {
    if (id < 1 || id > 4) return nullptr;
    return m_players[id - 1];
}

bool Bomberman::isPlayerAlive(int id) const {
    if (id < 1 || id > 4) return false;
    Player* p = m_players[id - 1];
    return p != nullptr && p->isAlive();
}

int Bomberman::countAlivePlayers() const {
    int count = 0;
    for (int id = 1; id <= 4; ++id) {
        Player* p = m_players[id - 1];
        if (p != nullptr && p->isAlive()) {
            ++count;
        }
    }
    return count;
}