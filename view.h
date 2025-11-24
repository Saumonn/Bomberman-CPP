#ifndef VIEW_H
#define VIEW_H

#include <string>
#include <ncurses.h>
#include "bomberman.h"

using namespace std;

class BombermanView {
    public:
        void init();
        void shutdown();

        void render(const Bomberman& model, const string& gameTitle, const string& levelName); // Crée la vue
        void displayFrame(const Bomberman& model, const string& gameTitle, const string& levelName); // Affiche la vue

        int getInput();

        void displayWin(int playerId);
        void displayLose(int playerId);
};

#endif // VIEW_H
