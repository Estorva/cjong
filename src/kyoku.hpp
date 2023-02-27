#ifndef KYOKU_HPP
#define KYOKU_HPP

#include <fstream>
#include <iomanip>
#include <nlohmann/json.hpp>
// Huge thanks to nlohmann for C++ JSON library!!
// https://github.com/nlohmann/json
#include <queue>
#include <stack>
#include <string>
#include <vector>
#include "constant.hpp" // K = 34
#include "conversion.hpp"
#include "hand.hpp"

using json = nlohmann::json;

enum Seat {
    JICHA,
    SHIMO,
    TOIMEN,
    KAMI
};

enum Naki {
    CHI,
    PON,
    ANKAN,
    KAKAN,
    MINKAN,
    RIICHI, // not technically naki but put here for convenience
    TSUMO,
    NO_NAKI
};

class Action {
public:
    Action() : agent(JICHA), patient(TOIMEN), naki(NO_NAKI), draw(-1), discard(-1), chi1(-1), chi2(-1) {;}

    Action(Seat a, Naki n, int dr, int dc) :
        agent(a), naki(n), draw(dr), discard(dc), patient(TOIMEN), chi1(-1), chi2(-1) {;}

    Action(Seat a, Naki n, int dr, int dc, Seat p, int c1, int c2) :
        agent(a), naki(n), draw(dr), discard(dc), patient(p), chi1(c1), chi2(c2) {;}

    std::string getStringAgent() {
        switch (agent) {
            case JICHA:
                return "Jicha";
            case SHIMO:
                return "Shimo";
            case TOIMEN:
                return "Toimen";
            case KAMI:
                return "Kami";
            // end case
        }
        return "Jicha";
    }
    std::string getStringNaki() {
        switch (naki) {
            case CHI:
                return "chiis";
            case PON:
                return "pons";
            case ANKAN:
                return "ankans";
            case KAKAN:
                return "kakans";
            case MINKAN:
                return "minkans";
            case RIICHI:
                return "riichis";
            case TSUMO:
            case NO_NAKI:
                return "draws";
            // end case
        }
        return "draws";
    }

    Seat agent;   // the player who makes this call
    Seat patient; // the player whose discard triggered this call
    Naki naki;
    int draw;
    int discard;
    int chi1; // tile used in chii
    int chi2; // tile used in chii
};

class SeatIndicator {
public:
    SeatIndicator() : _i(0) {;}
    SeatIndicator(int i) { (*this)(i); }
    SeatIndicator(Seat s) { (*this)(s); }

    Seat operator()() { return _intToSeat(_i); }
    SeatIndicator& operator()(Seat s) { _i = _seatToInt(s); return *this;}
    SeatIndicator& operator()(int i) { _i = i; return *this; }

    Seat operator++(int) { return _intToSeat(_i++); }
    Seat operator--(int) { return _intToSeat(_i--); }

    // move clockwise or counterclockwise
    Seat operator+(int i) { return _intToSeat(_i + i); }
    Seat operator-(int i) { return _intToSeat(_i - i); }

private:
    int  _seatToInt(Seat s) {
        int i = 0;
        switch (s) {
            case JICHA:
                i = 0;
                break;
            case SHIMO:
                i = 1;
                break;
            case TOIMEN:
                i = 2;
                break;
            case KAMI:
                i = 3;
                break;
        }
        return i;
    }
    Seat _intToSeat(int i) {
        Seat s = JICHA;
        // handle negative i
        // suppose i = -17
        // i % 4           = -1
        // i % 4 + 4       = 3
        // (i % 4 + 4) % 4 = 3
        i = (i % 4 + 4) % 4; 
        switch (i % 4) {
            case 0:
                s = JICHA;
                break;
            case 1:
                s = SHIMO;
                break;
            case 2:
                s = TOIMEN;
                break;
            case 3:
                s = KAMI;
                break;
        }
        return s;
    }

    int _i;
};

class Kyoku {
    // State of one kyoku
public:
    Kyoku() : hb(0), kn(0), kw(0) {
        for (int i = 0; i < 8; i++) {
            Hand h = Hand();
            hands.push_back(h);
        }
    }

    void        analyze(Seat);          // analyze the action history of a certain player
    std::string getTitle();             // returns string of kyoku wind and honba
    void        readJSON(json);         // reads JSON object given by class Game
    void        readMJLOG(std::string); // reads string in MJLOG format
    void        replay();               // print out the whole round

private:
    std::queue<int>    doraInd; // dora indicators
    std::vector<Hand>  hands;
    int                hb;      // honba aka bet bonus
    int                kn;      // kyoku number
    int                kw;      // kyoku wind, i.e. ton (0) or nan (1)
    Hand               pond;    // tiles thrown by players placed on the table
    std::stack<Action> sa;      // stack of actions
};

#endif
