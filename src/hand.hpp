#ifndef HAND_HPP
#define HAND_HPP

#include <algorithm>
#include <iostream>
#include <string>
#include <tuple>

#include "calsht_dw.hpp"
#include "conversion.hpp"

class Hand {
public:
    Hand() : arr(K, 0), discard(K, 0), numTile(0), numWait(0), shanten(0), wait(K, 0) {
        // K is 34 (= num of kinds of tiles) defined in constant.hpp
        // initialize arr as a vector of length K and with initial value 0
        hasAka5m = hasAka5p = hasAka5s = isAnalyzed = isChiitoi = false;
    }

    void        addTile(int);
    void        addTile(int, int);
    void        clear();
    std::vector<std::tuple<std::string, std::string>> getDiscard();
    std::vector<std::tuple<std::string, std::string, int>> getDiscard(Hand);
    std::string getMPSZ() const;
    int         getNumTile() const { return numTile; }
    int         getShanten();
    std::string getStringShanten();
    std::string getWait();
    int         getWaitNum() const { return numWait; }
    int         moda(int);
    int         moda(int, std::vector<int>);
    void        readMJLOG(std::string);       // read MJLOG string
    void        readMPSZ(std::string);        // read MPSZ encoding
    void        readTenhou(std::vector<int>); // read Tenhou encoding
    void        removeTile(int);
    void        removeTile(int, int);

    Hand        operator+(const Hand&) const;
    Hand        operator-(const Hand&) const;

    static CalshtDW csht;

private:
    std::vector<int> arr;
    std::vector<int> discard;
    bool             hasAka5m;
    bool             hasAka5p;
    bool             hasAka5s;
    bool             isAnalyzed;
    bool             isChiitoi;
    int              numTile;
    int              numWait;
    int              shanten;
    std::vector<int> wait;

    std::string      _33toMPSZ(const std::vector<int>&) const;
    std::string      _33toMPSZ(const int&) const;
    void             _analyze();
    void             _makeMax();
    std::vector<int> _MPSZto33(std::string) const;

    void _invalidHand(std::string h) {
        std::cout << "Invalid hand: " << h << std::endl;
        exit(1);
    }

    void _unknownChar(std::string h, char a) {
        std::cout << "Unknown char: `" << a << "' in hand " << h << std::endl;
        exit(1);
    }
};

#endif
