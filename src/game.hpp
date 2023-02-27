#ifndef GAME_HPP
#define GAME_HPP

#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include "hand.hpp"
#include "kyoku.hpp"

using json = nlohmann::json;

class Game {
public:
    Game() : vk(0), vn(4), j("") {;}
    void analyze();
    void analyze(size_t);
    void initialize();
    void read(std::string);
    void readJSON(std::string);
    void readMJLOG(std::string);
    void replay(int);
    void setJicha(std::string s) { j = s; }

private:
    std::vector<Kyoku*> vk;
    std::vector<std::string> vn;
    std::string j;
};

#endif
