#ifndef VIEW_H
#define VIEW_H

#ifdef OK
#undef OK
#endif

#ifdef Status
#undef Status
#endif

#include <string>
#include <map>
#include <opencv2/opencv.hpp>

#include "bomberman.h"

using namespace std;

class BombermanView {
public:
    BombermanView();

    void init();
    void shutdown();

    void render(const Bomberman& model,
                const string& gameTitle,
                const string& levelName);

    void displayFrame(const Bomberman& model,
                      const string& gameTitle,
                      const string& levelName);

    int getInput();

    void displayWin(int playerId);
    void displayLose(int playerId);

private:
    string m_windowName;
    string m_assetsDir;
    int    m_tileSize;
    cv::Mat m_canvas;
    map<char, cv::Mat> m_sprites;

    cv::Mat loadSprite(const string& filename);
    const cv::Mat& getSpriteForChar(char c) const;
};

#endif // VIEW_H
