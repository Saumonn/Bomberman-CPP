#include "view_opencv.h"

static const int TICK_MS = 100;

BombermanView::BombermanView()
    : m_windowName("Bomberman C++"),
      m_assetsDir("bmp"),
      m_tileSize(32),
      m_lastKey(-1)
{
}

cv::Mat BombermanView::loadSprite(const string& filename) {
    string fullPath = m_assetsDir + "/" + filename;
    cv::Mat img = cv::imread(fullPath, cv::IMREAD_UNCHANGED);
    if (img.empty()) {
        cerr << "Impossible de charger le sprite: " << fullPath << endl;
    }
    return img;
}

void BombermanView::init() {
    m_sprites['#'] = loadSprite("spriteWall.bmp");      // mur indestructible
    m_sprites['%'] = loadSprite("spriteSoftWall.bmp");  // mur destructible
    m_sprites['b'] = loadSprite("spriteBonus.bmp");     // bonus
    m_sprites['B'] = loadSprite("spriteBomb.bmp");      // bombe
    m_sprites['*'] = loadSprite("spriteFlame.bmp");     // feu
    m_sprites['1'] = loadSprite("sprite1.bmp");         // joueur 1
    m_sprites['2'] = loadSprite("sprite2.bmp");         // joueur 2
    m_sprites['3'] = loadSprite("sprite3.bmp");         // joueur 3
    m_sprites['4'] = loadSprite("sprite4.bmp");         // joueur 4

    m_sprites[' '] = cv::Mat(m_tileSize, m_tileSize, CV_8UC3, cv::Scalar(0, 0, 0));

    cv::namedWindow(m_windowName, cv::WINDOW_AUTOSIZE);
}

void BombermanView::shutdown() {
    cv::destroyWindow(m_windowName);
}

const cv::Mat& BombermanView::getSpriteForChar(char c) const {
    auto it = m_sprites.find(c);
    if (it != m_sprites.end() && !it->second.empty()) {
        return it->second;
    }

    auto itEmpty = m_sprites.find(' ');
    if (itEmpty != m_sprites.end() && !itEmpty->second.empty()) {
        return itEmpty->second;
    }

    static cv::Mat fallback(32, 32, CV_8UC3, cv::Scalar(0, 0, 0));
    return fallback;
}

void BombermanView::render(const Bomberman& model, const string& gameTitle, const string& levelName)
{
    int width  = model.getWidth();
    int height = model.getHeight();

    int hudHeight = 80; // espace pour afficher les infos en dessous
    int canvasWidth  = width  * m_tileSize;
    int canvasHeight = height * m_tileSize + hudHeight;

    // Créer / redimensionner le canvas
    m_canvas = cv::Mat(canvasHeight, canvasWidth, CV_8UC3, cv::Scalar(0, 0, 0));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {

            char c;

            if (model.hasBombAt(x, y)) {
                c = 'B';
            } else {
                Element* e = model.getCell(x, y);
                c = e->getSymbol();
            }

            const cv::Mat& sprite = getSpriteForChar(c);
            if (!sprite.empty()) {
                cv::Rect roi(x * m_tileSize, y * m_tileSize, m_tileSize, m_tileSize);
                if (sprite.cols != m_tileSize || sprite.rows != m_tileSize) {
                    cv::Mat resized;
                    cv::resize(sprite, resized, cv::Size(m_tileSize, m_tileSize));
                    resized.copyTo(m_canvas(roi));
                } else {
                    sprite.copyTo(m_canvas(roi));
                }
            }
        }
    }

    // --- HUD (texte) ---
    int hudY = height * m_tileSize + 20;
    cv::putText(m_canvas,
                gameTitle + " - " + levelName,
                cv::Point(10, hudY),
                cv::FONT_HERSHEY_SIMPLEX,
                0.6,
                cv::Scalar(255, 255, 255),
                1);

    // Infos joueurs
    int baseY = hudY + 20;
    for (int id = 1; id <= 4; ++id) {
        Player* p = model.getPlayer(id);
        int baseX = 10 + (id - 1) * 150;

        if (p == nullptr) {
            cv::putText(m_canvas,
                        "P" + to_string(id) + ": ---",
                        cv::Point(baseX, baseY),
                        cv::FONT_HERSHEY_SIMPLEX,
                        0.5,
                        cv::Scalar(200, 200, 200),
                        1);
            continue;
        }

        string status = p->isAlive() ? "VIVANT" : "MORT";
        string txt = "P" + to_string(id) +
                     " [" + status + "] Pow=" + to_string(p->getPower());

        cv::putText(m_canvas,
                    txt,
                    cv::Point(baseX, baseY),
                    cv::FONT_HERSHEY_SIMPLEX,
                    0.5,
                    cv::Scalar(200, 200, 200),
                    1);
    }

    // Aide commandes
    cv::putText(m_canvas,
                "ZQSD: deplacement  B: bombe  X: quitter",
                cv::Point(10, canvasHeight - 10),
                cv::FONT_HERSHEY_SIMPLEX,
                0.5,
                cv::Scalar(180, 180, 180),
                1);

    cv::imshow(m_windowName, m_canvas);
}

void BombermanView::displayFrame(const Bomberman& model,
                                 const string& gameTitle,
                                 const string& levelName)
{
    render(model, gameTitle, levelName);
    m_lastKey = cv::waitKey(TICK_MS);
}

int BombermanView::getInput() {
    int key = m_lastKey;
    m_lastKey = -1;
    return key;
}

void BombermanView::displayWin(int playerId) {
    int w = 640;
    int h = 200;
    cv::Mat endScreen(h, w, CV_8UC3, cv::Scalar(0, 100, 0));

    string msg = "LE JOUEUR " + to_string(playerId) + " A GAGNE !";
    cv::putText(endScreen, msg,
                cv::Point(40, 80),
                cv::FONT_HERSHEY_SIMPLEX,
                1.0,
                cv::Scalar(255, 255, 255),
                2);

    cv::putText(endScreen,
                "Appuyez sur une touche pour quitter...",
                cv::Point(40, 140),
                cv::FONT_HERSHEY_SIMPLEX,
                0.6,
                cv::Scalar(230, 230, 230),
                1);

    cv::imshow(m_windowName, endScreen);
    cv::waitKey(0);
}

void BombermanView::displayLose(int playerId) {
    int w = 640;
    int h = 200;
    cv::Mat endScreen(h, w, CV_8UC3, cv::Scalar(0, 0, 80));

    string msg = "LE JOUEUR " + to_string(playerId) + " EST MORT...";
    cv::putText(endScreen, msg,
                cv::Point(40, 80),
                cv::FONT_HERSHEY_SIMPLEX,
                1.0,
                cv::Scalar(255, 255, 255),
                2);

    cv::putText(endScreen,
                "Appuyez sur une touche pour quitter...",
                cv::Point(40, 140),
                cv::FONT_HERSHEY_SIMPLEX,
                0.6,
                cv::Scalar(230, 230, 230),
                1);

    cv::imshow(m_windowName, endScreen);
    cv::waitKey(0);
}
