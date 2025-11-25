#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>

#include "bomberman.h"
#include "levels.h"
#include "view_opencv.h"
#include "controller.h"

using namespace std;

int main(int argc, char** argv) {

    int levelNumber = 1;
    int playerId    = 1;
    string serverIp = "127.0.0.1";
    int serverPort  = 4000;

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];

        if ((arg == "-l") && i + 1 < argc) {
            levelNumber = atoi(argv[++i]);
        }
        else if ((arg == "-p") && i + 1 < argc) {
            playerId = atoi(argv[++i]);
        }
        else if ((arg == "-s") && i + 1 < argc) {
            serverIp = argv[++i];
        }
        else if ((arg == "-P") && i + 1 < argc) {
            serverPort = atoi(argv[++i]);
        }
        else {
            cout << "Usage: " << argv[0] << " [-l niveau(1-5)] [-p joueur(1-4)] [-s ip] [-P port]\n";
            return 0;
        }
    }

    if (levelNumber < 1 || levelNumber > 5) levelNumber = 1;
    if (playerId    < 1 || playerId    > 4) playerId    = 1;

    const vector<const vector<string>*> levelTable = {&level1, &level2, &level3, &level4, &level5};

    const vector<string>& selectedLevel = *levelTable[levelNumber - 1];
    string levelName = "Niveau " + to_string(levelNumber);

    Bomberman model;
    BombermanView view;

    model << selectedLevel;

    BombermanController controller(model, view, "Bomberman C++", levelName, playerId, serverIp, serverPort, true);

    controller.run();
    return 0;
}
