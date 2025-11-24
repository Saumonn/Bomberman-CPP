#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <string>
#include <ncurses.h>
#include "bomberman.h"
#include "view_opencv.h"

using namespace std;

class BombermanController {
    public:
        BombermanController(Bomberman& model, BombermanView& view, const string& gameTitle, const string& levelName, int playerId);

        void run();

    private:
        Bomberman& m_model;
        BombermanView& m_view;
        string m_gameTitle;
        string m_levelName;
        int m_playerId;
};

#endif // CONTROLLER_H
