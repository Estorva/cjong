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

class Seat {
    // Class for seat (E, S, W, N)
    // Using an int to store information and derive seat by modulo 4
    // 0 defaults to chicha
public:
    Seat() : _i(0) {;}
    Seat(int i) { (*this)(i); }
    Seat(const Seat& s) { _i = s._i; }

    Seat operator=(const Seat& s) { _i = s._i; return *this; }

    // return int presentation of Seat object
    int operator()() const { return (_i % 4 + 4) % 4; }

    // set Seat object as other objects or int
    Seat& operator()(const Seat& s) { _i = s._i; return *this;}
    Seat& operator()(int i) { _i = i; return *this; }

    // increment or decrement
    int operator++(int) { return (_i++ % 4 + 4) % 4; }
    int operator--(int) { return (_i-- % 4 + 4) % 4; }

    // move clockwise or counterclockwise
    int operator+(int i) const { return ((_i + i) % 4 + 4) % 4; }
    int operator-(int i) const { return ((_i - i) % 4 + 4) % 4; }

    // compare
    bool operator==(const Seat& s) const { return (_i - s._i) % 4 == 0; }
    bool operator==(int i) const { return (_i - i) % 4 == 0; }
    bool operator!=(const Seat& s) const { return (_i - s._i) % 4 != 0; }
    bool operator!=(int i) const { return (_i - i) % 4 != 0; }

    std::string getString() const {
        switch ((_i % 4 + 4) % 4) {
            case 0:
                return "East";
            case 1:
                return "South";
            case 2:
                return "West";
            case 3:
                return "North";
        }
        return "";
    }

    static Seat EAST;
    static Seat SOUTH;
    static Seat WEST;
    static Seat NORTH;

private:
    int _i;
};

class Action {
public:
    Action() : agent(Seat::EAST), patient(Seat::EAST), naki(NO_NAKI), draw(-1), discard(-1), chi1(-1), chi2(-1) {;}

    Action(Seat a, Naki n, int dr, int dc) :
        agent(a), patient(Seat::EAST), naki(n), draw(dr), discard(dc), chi1(-1), chi2(-1) {;}

    Action(Seat a, Naki n, int dr, int dc, Seat p, int c1, int c2) :
        agent(a), patient(p), naki(n), draw(dr), discard(dc), chi1(c1), chi2(c2) {;}

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
