#include "controller.h"

#include <sstream>
#include <stdexcept>

using namespace std;

BombermanController::BombermanController(Bomberman& model,
                                         BombermanView& view,
                                         const string& gameTitle,
                                         const string& levelName,
                                         int playerId,
                                         const string& serverIp,
                                         int serverPort,
                                         bool enableNetwork)
    : m_model(model),
      m_view(view),
      m_gameTitle(gameTitle),
      m_levelName(levelName),
      m_playerId(playerId),
      m_networkEnabled(enableNetwork),
      m_serverIp(serverIp),
      m_serverPort(serverPort) {}

BombermanController::~BombermanController() { stopNetwork(); }

void BombermanController::startNetwork() {
    if (!m_networkEnabled) return;
    try {
        m_socket.socketConnect("game", m_serverIp, static_cast<uint16_t>(m_serverPort));
        m_runningNetwork = true;
        m_recvThread = std::thread(&BombermanController::networkLoop, this);
        // annoncer sa présence
        std::ostringstream oss;
        oss << "JOIN " << m_playerId << "\n";
        sendAction(oss.str());
    } catch (const std::exception& e) {
        m_networkEnabled = false;
        cerr << "Connexion réseau désactivée: " << e.what() << endl;
    }
}

void BombermanController::stopNetwork() {
    if (!m_networkEnabled) return;
    m_runningNetwork = false;
    if (m_recvThread.joinable()) {
        m_recvThread.join();
    }
    m_socket.closeConnection("game");
}

void BombermanController::networkLoop() {
    while (m_runningNetwork) {
        try {
            string data = m_socket.socketReceive("game");
            if (data.empty()) {
                break; // connexion fermée
            }

            // Assemblage ligne par ligne
            m_partialBuffer.append(data);
            size_t pos;
            while ((pos = m_partialBuffer.find('\n')) != string::npos) {
                string line = m_partialBuffer.substr(0, pos);
                m_partialBuffer.erase(0, pos + 1);
                {
                    lock_guard<mutex> lock(m_queueMutex);
                    m_incomingLines.push_back(line);
                }
            }
        } catch (const std::exception&) {
            break;
        }
    }
    m_runningNetwork = false;
}

void BombermanController::sendAction(const string& line) {
    if (!m_networkEnabled || !m_runningNetwork) return;
    try {
        m_socket.socketSend("game", line);
    } catch (const std::exception& e) {
        cerr << "Erreur d'envoi réseau: " << e.what() << endl;
    }
}

void BombermanController::applyActionLine(const string& line) {
    // Format attendu: ACT <id> <cmd>
    std::istringstream iss(line);
    string tag;
    iss >> tag;
    if (tag != "ACT") return;
    int pid;
    string cmd;
    if (!(iss >> pid >> cmd)) return;
    if (pid < 1 || pid > 4) return;

    if (cmd == "U") {
        m_model.movePlayer(pid, 0, -1);
    } else if (cmd == "D") {
        m_model.movePlayer(pid, 0, 1);
    } else if (cmd == "L") {
        m_model.movePlayer(pid, -1, 0);
    } else if (cmd == "R") {
        m_model.movePlayer(pid, 1, 0);
    } else if (cmd == "B") {
        m_model.placeBomb(pid);
    } else if (cmd == "X") {
        // rien de spécial côté modèle, mais on peut arrêter si c'est nous
        if (pid == m_playerId) {
            m_runningNetwork = false;
        }
    }
}

void BombermanController::run() {
    m_view.init();

    startNetwork();

    bool running = true;

    while (running) {

        int key = m_view.getInput();

        // Entrée locale -> message réseau ou action directe
        string out;
        switch (key) {
            case 'z': out = "ACT " + to_string(m_playerId) + " U\n"; break;
            case 's': out = "ACT " + to_string(m_playerId) + " D\n"; break;
            case 'q': out = "ACT " + to_string(m_playerId) + " L\n"; break;
            case 'd': out = "ACT " + to_string(m_playerId) + " R\n"; break;
            case 'b': out = "ACT " + to_string(m_playerId) + " B\n"; break;
            case 'x': out = "ACT " + to_string(m_playerId) + " X\n"; running = false; break;
        }

        if (!out.empty()) {
            if (m_networkEnabled && m_runningNetwork) {
                sendAction(out);
            } else {
                // mode local fallback
                applyActionLine(out);
            }
        }

        // Appliquer les actions reçues
        {
            lock_guard<mutex> lock(m_queueMutex);
            for (const auto& line : m_incomingLines) {
                applyActionLine(line);
            }
            m_incomingLines.clear();
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
