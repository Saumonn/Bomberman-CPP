#include <ncurses.h>
#include "view.h"

static const int TICK_MS = 100;
static const int HUD_SPACE = 25;

namespace {
    int colorPair(char c) {
        switch (c) {
            case ' ': return 1;  // vide
            case '#': return 2;  // mur indestructible
            case '%': return 3;  // mur destructible
            case 'b': return 4;  // bonus
            case '*': return 6;  // feu
            case '1': return 7;  // joueur 1
            case '2': return 8;  // joueur 2
            case '3': return 9;  // joueur 3
            case '4': return 10; // joueur 4
            default:  return 1;  // défaut
        }
    }
}

void BombermanView::init() {
    initscr();
    curs_set(0);
    noecho();
    keypad(stdscr, TRUE);
    start_color();

    timeout(TICK_MS);

    init_pair(1, COLOR_WHITE,  COLOR_BLACK);   // vide
    init_pair(2, COLOR_WHITE,  COLOR_BLACK);   // mur indestructible
    init_pair(3, COLOR_WHITE,  COLOR_BLACK);   // mur destructible
    init_pair(4, COLOR_BLUE,   COLOR_BLACK);   // bonus
    init_pair(5, COLOR_RED,    COLOR_BLACK);   // bombe
    init_pair(6, COLOR_YELLOW, COLOR_RED);     // feu
    init_pair(7, COLOR_BLACK,  COLOR_GREEN);   // joueur 1
    init_pair(8, COLOR_BLACK,  COLOR_MAGENTA); // joueur 2
    init_pair(9, COLOR_WHITE,  COLOR_BLUE);    // joueur 3
    init_pair(10, COLOR_WHITE, COLOR_YELLOW);  // joueur 4
}

void BombermanView::shutdown() {
    curs_set(1);
    endwin();
}

void BombermanView::render(const Bomberman& model, const string& gameTitle, const string& levelName) {
    int width  = model.getWidth();
    int height = model.getHeight();

    attron(A_BOLD);
    mvprintw(0, 0, "%s - %s", gameTitle.c_str(), levelName.c_str());
    attroff(A_BOLD);

    for (int x = 0; x < width; ++x) {
        mvaddch(1, x, '-');
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {

            char c;
            int pair = 1;

            if (model.hasBombAt(x, y)) { // Prioritaire
                c = 'B';
                pair = 5;
            } else {
                Element* e = model.getCell(x, y);
                c = e->getSymbol();
                pair = colorPair(c);
            }

            attron(COLOR_PAIR(pair));
            mvaddch(y + 2, x, c);
            attroff(COLOR_PAIR(pair));
        }
    }

    int hudRow = height + 3;

    for (int id = 1; id <= 4; ++id) {
        Player* p = model.getPlayer(id);
        int col = (id - 1) * HUD_SPACE;

        if (p == nullptr) {
            mvprintw(hudRow, col, "P%d: ---", id);
            continue;
        }

        bool alive = p->isAlive();
        int  power = p->getPower();

        int colorPair = 1;
        if      (id == 1) colorPair = 7;
        else if (id == 2) colorPair = 8;
        else if (id == 3) colorPair = 9;
        else if (id == 4) colorPair = 10;

        attron(COLOR_PAIR(colorPair) | A_BOLD);
        mvprintw(hudRow, col, "P%d [%s] Power=%d", id, alive ? "VIVANT" : "MORT", power);
        attroff(COLOR_PAIR(colorPair) | A_BOLD);
    }

    mvprintw(hudRow + 2, 0, "ZQSD pour se deplacer, B pour bombe, X pour quitter...");
}

void BombermanView::displayFrame(const Bomberman& model, const string& gameTitle, const string& levelName) {
    clear();
    render(model, gameTitle, levelName);
    refresh();
}

int BombermanView::getInput() {
    return getch();
}

void BombermanView::displayWin(int playerId) {
    clear();
    attron(A_BOLD | COLOR_PAIR(7));
    mvprintw(5, 5, "LE JOUEUR %d A GAGNE !", playerId);
    attroff(A_BOLD | COLOR_PAIR(7));

    mvprintw(7, 5, "Appuyez sur une touche pour quitter...");
    refresh();

    timeout(-1);
    getch();
}

void BombermanView::displayLose(int playerId) {
    clear();
    attron(A_BOLD | COLOR_PAIR(5));
    mvprintw(5, 5, "LE JOUEUR %d EST MORT !", playerId);
    mvprintw(6, 5, "DEFAITE...");
    attroff(A_BOLD | COLOR_PAIR(5));

    mvprintw(8, 5, "Appuyez sur une touche pour quitter...");
    refresh();

    timeout(-1);
    getch();
}
