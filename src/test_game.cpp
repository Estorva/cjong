// g++ --std=c++17 -I/opt/local/include game.cpp kyoku.cpp hand.cpp calsht_dw.cpp test_game.cpp -o build/test_game

#include <iostream>
#include "game.hpp"

int main(int argc, char** argv) {
    // usage:
    //     test_game
    //         Using default log (gamelog/2023_1_1_Jade_Room_South.json) and
    //         analyzes first round.
    //     test_game <file>
    //         Analyzing all rounds in the log <file>
    //     test_game <file> <# of round>
    //         Analyzing the specified round of <file>
    Hand::csht.initialize("index");
    Game g;

    switch (argc) {
        case 1:
            //g.readJSON("gamelog/2023_1_1_Jade_Room_South.json"); // load MJS log
            g.readMJLOG("gamelog/2023020601gm-0009-0000-33fbd68e.log"); // load MJLOG log
            g.analyze(0);
            break;
        case 2:
            g.readMJLOG(argv[1]);
            g.analyze();
            break;
        case 3:
            g.readJSON(argv[1]);
            g.analyze(std::atoi(argv[2]));
            break;
    }


    return 0;
}
