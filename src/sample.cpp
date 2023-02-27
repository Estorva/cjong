#include <iostream>
#include <fstream>
#include <iomanip>
#include <array>
#include <random>
#include <utility>
#include <algorithm>
#include <chrono>
#include <bitset>
#include <cstdint>
#include "calsht_dw.hpp"

// $ ./sample 1000000
// $ cat result.txt
// Number of Tiles         14
// Total                   1000000
// Time (msec.)            106499
//
// Turn    Shanten Number (-1 - 6)                                         Hora    Tempai  Exp.
// 0       4       689     23224   194623  439207  285376  55254   1623    4e-06   0.000693        3.1576
// 1       36      3139    60055   312301  436206  169174  18856   233     3.6e-05 0.003175        2.76561
// 2       165     9550    118653  409619  368079  87961   5939    34      0.000165        0.009715        2.42371
// 3       575     22236   193284  461052  278799  42243   1805    6       0.000575        0.022811        2.12924
// 4       1585    43455   270484  467989  196448  19497   539     3       0.001585        0.04504 1.87492
// 5       3557    73558   341680  439988  132134  8916    167     0       0.003557        0.077115        1.651
// 6       7014    111731  397628  393033  86425   4115    54      0       0.007014        0.118745        1.45269
// 7       12342   155439  437312  337357  55610   1918    22      0       0.012342        0.167781        1.2743
// 8       19769   202505  460160  281105  35463   991     7       0       0.019769        0.222274        1.11299
// 9       29749   250970  466746  229544  22484   502     5       0       0.029749        0.280719        0.96557
// 10      42291   298561  460318  184296  14284   247     3       0       0.042291        0.340852        0.830474
// 11      57229   343412  444338  145635  9257    128     1       0       0.057229        0.400641        0.706667
// 12      74977   383949  420945  114180  5875    74      0       0       0.074977        0.458926        0.592249
// 13      94703   420040  392750  88629   3832    46      0       0       0.094703        0.514743        0.486985
// 14      116313  450680  362048  68448   2487    24      0       0       0.116313        0.566993        0.390188
// 15      140202  475546  330180  52435   1622    15      0       0       0.140202        0.615748        0.299774
// 16      165530  494708  298847  39820   1087    8       0       0       0.16553 0.660238        0.21625
// 17      192032  508775  268240  30169   780     4       0       0       0.192032        0.700807        0.138902

int main(int argc, char* argv[])
{
    if (argc != 2) {
        return 1;
    }

#ifdef THREE_PLAYER
    const int T = 27;
#else
    const int T = 34;
#endif
    const int M = 14;
    const int N = std::atoi(argv[1]);
    const int MODE = 7;

    std::vector<int> hd(K, 0);
    std::array<int, K> wl;
    std::array<int, K> cnt;
    std::array<int, 4 * T> wall;
    int table[18][8] = {};
    CalshtDW calsht;

    calsht.initialize(INDEX_FILE_PATH);

    std::mt19937_64 rand(std::random_device{}());
    std::ofstream fout("result.txt");

    auto itr = wall.begin();

    for (int i = 0; i < K; ++i) {
#ifdef THREE_PLAYER
        if (i > 0 && i < 8) continue;
#endif
        for (int j = 0; j < 4; ++j) {
            *itr++ = i;
        }
    }

    auto start = std::chrono::system_clock::now();

    for (int i = 0; i < N; ++i) {
        std::fill(hd.begin(), hd.end(), 0);
        std::fill(wl.begin(), wl.end(), 4);

        for (int j = 1; j <= M + 17; ++j) {
            int n = rand() % (4 * T - j + 1);
            ++hd[wall[n]];
            --wl[wall[n]];
            std::swap(wall[n], wall[4 * T - j]);

            if (j >= M) {
            auto [sht, mode, disc, wait] = calsht(hd, M / 3, MODE);
            ++table[j - M][sht];

                if (sht > 0) {
                    std::bitset<K> bs1(disc);
                    cnt.fill(-1);

                    for (int k = 0; k < K; ++k) {
                        if (bs1[k]) {
                            --hd[k];
                            auto [sht_, mode_, disc_, wait_] = calsht(hd, M / 3, MODE);
                            std::bitset<K> bs2(wait_);
                            cnt[k] = 0;

                            for (int l = 0; l < K; ++l) {
                                cnt[k] += bs2[l] ? wl[l] : 0;
                            }
                            ++hd[k];
                        }
                    }
                    n = rand() % K;
                    int tile = n;
                    int max = cnt[n];

                    for (int k = n + 1; k < n + K; ++k) {
                        if (cnt[k % K] > max) {
                            tile = k % K;
                            max = cnt[k % K];
                        }
                    }
                    --hd[tile];
                }
                else {
                    for (int k = j + 1; k <= M + 17; ++k) {
                        ++table[k - M][sht];
                        n = rand() % (4 * T - k + 1);
                        std::swap(wall[n], wall[4 * T - k]);
                    }
                    break;
                }
            }
        }
    }

    auto end = std::chrono::system_clock::now();

    fout.setf(std::ios::left, std::ios::adjustfield);
    fout << std::setw(24) << "Number of Tiles" << std::setw(16) << M << '\n';
    fout << std::setw(24) << "Total" << std::setw(16) << N << '\n';
    fout << std::setw(24) << "Time (msec.)" << std::setw(16) << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << '\n';
    fout << std::setw(70) << "\nTurn\tShanten Number (-1 - 6)"
         << "Hora\tTempai\tExp.\n";

    for (int i = 0; i < 18; ++i) {
        fout << i << '\t';
        double ev = 0;

        for (int j = 0; j < 8; ++j) {
            ev += (j - 1) * table[i][j];
            fout << table[i][j] << '\t';
        }
        fout << 1.0 * table[i][0] / N << '\t' << 1.0 * (table[i][0] + table[i][1]) / N << '\t' << ev / N << '\n';
    }

    return 0;
}
