#include "controller.h"

using namespace std;

BombermanController::BombermanController(Bomberman& model, BombermanView& view, const string& gameTitle, const string& levelName, int playerId) : 
    m_model(model), m_view(view), m_gameTitle(gameTitle), m_levelName(levelName), m_playerId(playerId) { }

void BombermanController::run() {
    m_view.init();
    bool running = true;

    while (running) {

        int key = m_view.getInput();

        switch (key) {
            case 'z': m_model.movePlayer(m_playerId, 0, -1); break;
            case 's': m_model.movePlayer(m_playerId, 0, 1);  break;
            case 'q': m_model.movePlayer(m_playerId, -1, 0); break;
            case 'd': m_model.movePlayer(m_playerId, 1, 0);  break;
            case 'b': m_model.placeBomb(m_playerId);         break;
            case 'x': running = false;                       break;
        }

        m_model.update();

        if (!m_model.isPlayerAlive(m_playerId)) {
            m_view.displayLose(m_playerId);
            break;
        }

        if (m_model.countAlivePlayers() <= 1 && m_model.isPlayerAlive(m_playerId)) {
            m_view.displayWin(m_playerId);
            break;
        }

        m_view.displayFrame(m_model, m_gameTitle, m_levelName);
    }
    m_view.shutdown();
}
