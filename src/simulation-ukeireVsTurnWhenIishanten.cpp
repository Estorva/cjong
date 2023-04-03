// A backup of simulation.cpp
// There is no target for this file in Makefile - rename it to simulation.cpp
// to compile this file.
#include <chrono>
#include <iostream>
#include <iomanip>
#include <random>

#include "calsht_dw.hpp"
#include "hand.hpp"

int main(int argc, char** argv)
{
    if (argc != 2) {
        return 1;
    }

    auto start = std::chrono::system_clock::now();

    // simulate greedy algorithm of a hand
    std::mt19937_64 rand(std::random_device{}());
    Hand::csht.initialize("index");

    Hand h;
    std::vector<int> wall(K, 4);
    // widest iishanten is 44456m4567p4567s (18 kinds, max 61 tiles)
    std::vector<std::vector<int>> istMat(70, std::vector<int>(0));
    std::vector<float> istAvg(70, 0);

    for (int t = 0; t < std::atoi(argv[1]); t++) {
        // t = num of trial
        h.clear();
        std::fill(wall.begin(), wall.end(), 4);

        // randomly draw 13 tiles from wall
        for (int i = 0;;) {
            int tsumo = rand() % K;
            if (wall[tsumo] == 0) continue;
            wall[tsumo]--;
            h.addTile(tsumo);
            i++;
            if (i == 13) break;
        }

        // remove 13*3 tiles for other players and 14 for dead wall
        for (int i = 0;;) {
            int tsumo = rand() % K;
            if (wall[tsumo] == 0) continue;
            wall[tsumo]--;
            i++;
            if (i == 53) break;
        }

        // flatten wall
        std::vector<int> w(0);
        for (int i = 0; i < K; i++) {
            for (int j = 0; j < wall[i]; j++) {
                w.push_back(i);
            }
        }
        std::shuffle(w.begin(), w.end(), rand);

        int istTurn = 0;
        int istWait = 0;
        bool istLock = false;
        for (int i = 0; i < 18; i++) {
            // 18 is a rough estimation of how many turns a player gets to draw in a round
            int tsumo = w[0];
            w.erase(w.begin());
            h.addTile(tsumo);
            if (h.getShanten() <= 0) {
                // only record turn number when reaches tenpai
                istMat[istWait].push_back(istTurn);
                break;
            }

            if (h.getShanten() != 1 || (h.getShanten() == 1 && !istLock)) {
                h.removeTile(tsumo);
                int discard = h.moda(tsumo);

                if (h.getShanten() == 1) istLock = true;
            }
            else {
                // for simulation sake, lock in 1 shanten
                h.removeTile(tsumo);
                istTurn++;
                istWait = h.getWaitNum();
            }
        }
    }

    auto end = std::chrono::system_clock::now();

    std::cout.setf(std::ios::left, std::ios::adjustfield);
    std::cout << std::setw(13) << "Time (msec.)" << std::setw(16) << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << '\n';
    std::cout << std::setw(13) << "# of Waits" << std::setw(8) << "Average"
              << std::setw(5) << "Min" << std::setw(5) << "Q1"
              << std::setw(5) << "Q2" << std::setw(5) << "Q3"
              << std::setw(5) << "Max" << std::setw(13) << "# of Samples" << '\n';

    for (int i = 1; i < 70; i++) {
        // average turn for each wait tile number
        std::vector<int> istTurnVec = istMat[i];
        if (istTurnVec.size() == 0) continue;

        // avg
        int sum = 0;
        for (auto it = istTurnVec.begin(); it != istTurnVec.end(); it++) sum += *it;
        float avg = (float)sum / (float)istTurnVec.size();

        // Q1 Q2 and Q3
        std::sort(istTurnVec.begin(), istTurnVec.end());

        std::cout << std::setw(13) << i << std::setw(8) << std::setprecision(5)
                  << avg << std::setw(5) << istTurnVec[0]
                  << std::setw(5) << istTurnVec[istTurnVec.size() / 4]
                  << std::setw(5) << istTurnVec[istTurnVec.size() / 2]
                  << std::setw(5) << istTurnVec[istTurnVec.size() * 3 / 4]
                  << std::setw(5) << istTurnVec[istTurnVec.size() - 1]
                  << std::setw(13) << istTurnVec.size() << '\n';
    }



    return 0;
}
