#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "bomberman.h"
#include "easysocket.hpp"
#include "view_opencv.h"

using namespace std;

class BombermanController {
    public:
        BombermanController(Bomberman& model,
                            BombermanView& view,
                            const string& gameTitle,
                            const string& levelName,
                            int playerId,
                            const string& serverIp,
                            int serverPort,
                            bool enableNetwork);

        ~BombermanController();

        void run();

    private:
        Bomberman& m_model;
        BombermanView& m_view;
        string m_gameTitle;
        string m_levelName;
        int m_playerId;

        // Réseau
        bool m_networkEnabled;
        string m_serverIp;
        int m_serverPort;
        masesk::EasySocket m_socket;
        std::thread m_recvThread;
        std::atomic<bool> m_runningNetwork{false};
        std::mutex m_queueMutex;
        std::vector<string> m_incomingLines;
        string m_partialBuffer;

        void startNetwork();
        void stopNetwork();
        void networkLoop();
        void sendAction(const string& line);
        void applyActionLine(const string& line);
};

#endif // CONTROLLER_H
