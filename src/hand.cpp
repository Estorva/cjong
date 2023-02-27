#include "hand.hpp"

/******************************************************************************\
 * std::tuple<int, int, int64_t, int64_t> CalshtDw::operator()
 *     (const std::vector<int>& t, int m, int mode) const;
 * Input:
 *     t:    A vector of integers representing the number of each kind of tile
 *           in a hand.
 *     m:    Number of groups of 3. Equals to size of hand (excluding melds)
 *           divided by 3.
 *     mode: One of the following or logical OR of any of the following:
 *           - 1: general form (not considering chiitoi and kokushi)
 *           - 2: chiitoi
 *           - 4: kokushi
 * Output:
 *     Let auto [sht, mode, disc, wait] = std::tuple<int, int, int64_t, int64_t>
 *     sht:  Number of shanten (least number of draws to get to tenpai) + 1.
 *     mode: nearest mode (general form/chiitoi/kokushi).
 *     disc: a 64 bit integer with its i-th bit representing whether discarding
 *           the i-th tile(1) does not decrease shanten.
 *     wait: a 64 bit integer with its i-th bit representing whether drawing
 *           the i-th tile decreases shanten.
 *
 * (1) the indexing of tile is based on the following scheme:
 *     0-8: 1m-9m, 9-17: 1p-9p, 18-26: 1s-9s, 27-33: ton, sha, nan, pei, haku, hatsu, chun
\******************************************************************************/

void Hand::addTile(int i33) {
    if (i33 < 0 || 33 < i33) return;
    if (arr[i33] < 4) {
        arr[i33]++;
        numTile++;
        isAnalyzed = false;
    }
}

void Hand::addTile(int i33, int times) {
    while (0 <= i33 && i33 <= 33 && times > 0 && arr[i33] < 4) {
        arr[i33]++;
        numTile++;
        times--;
        isAnalyzed = false;
    }
    return;
}

void Hand::clear() {
    std::fill(arr.begin(), arr.end(), 0);
    std::fill(discard.begin(), discard.end(), 0);
    std::fill(wait.begin(), wait.end(), 0);
    hasAka5m = hasAka5p = hasAka5s = isAnalyzed = isChiitoi = false;
    numTile = numWait = shanten = 0;
}

std::vector<std::tuple<std::string, std::string>> Hand::getDiscard() {
    // return a vector of choices of discards and their ukeire
    // ***not sorted
    // only works when hand has tile 3n+2, otherwise return empty vector
    if (numTile % 3 == 1) return std::vector<std::tuple<std::string, std::string>>(0);

    if (!isAnalyzed) _analyze();

    std::vector<std::tuple<std::string, std::string>> result(0);
    for (int i = 0; i < K; i++) {
        if (discard[i] > 0) {
            // this tile is marked as can be discarded
            // create a copy of `arr' with this kind of tile subtracted
            std::vector<int> _arr(arr);
            _arr[i] -= 1;
            auto [s, m, d, w] = csht(_arr, numTile / 3, 7);

            std::vector<int> v(K, 0);
            int j = 0;
            while (w != 0) {
                v[j] = w % 2;
                w = w / 2;
                j++;
            }

            std::tuple<std::string, std::string> p(_33toMPSZ(i), _33toMPSZ(v));
            result.push_back(p);
        }
    }
    return result;
}

std::vector<std::tuple<std::string, std::string, int>> Hand::getDiscard(Hand vis) {
    // get best discard judging by visible tiles
    if (!isAnalyzed) _analyze();
    std::vector<std::tuple<std::string, std::string>> disMax = getDiscard();
    std::vector<std::tuple<std::string, std::string, int>> result(0);

    Hand hw, dis;
    for (auto tss : disMax) {
        auto [sd, sw] = tss;
        hw.readMPSZ(sw);
        hw._makeMax();
        dis = (hw - vis) - (*this); // maximum wait minus visible tiles and self hand
        result.push_back(std::make_tuple(sd, sw, dis.getNumTile()));
    }

    std::sort(result.begin(), result.end(),
        [](const auto& t1, const auto& t2) {
            return std::get<2>(t1) < std::get<2>(t2) ||
                   (std::get<2>(t1) == std::get<2>(t2) && std::get<1>(t1).size() < std::get<1>(t2).size()) ||
                   (std::get<2>(t1) == std::get<2>(t2) && std::get<1>(t1).size() == std::get<1>(t2).size() &&
                    cjong::MPSZto33(std::get<0>(t1)) > cjong::MPSZto33(std::get<0>(t2)));
        }
    ); // sorting using lambda, compare 2nd element; if equal, compare wait string length
    std::reverse(result.begin(), result.end()); // first entry has widest wait

    return result;
}

std::string Hand::getMPSZ() const {
    return _33toMPSZ(arr);
}

int Hand::getShanten() {
    if (!isAnalyzed) _analyze();

    return shanten;
}

std::string Hand::getStringShanten() {
    int st = getShanten();
    if (st == -1) return "hora";
    if (st == 0)  return "tenpai";

    return std::to_string(st) + " shanten";
}

std::string Hand::getWait() {
    if (!isAnalyzed) _analyze();

    return _33toMPSZ(wait);
}

int Hand::moda(int tsumo) {
    // simulates the action of moda (tsumo (draw) and dahai (discard))
    // returns the tile to be discarded according to greatest ukieire
    // when no context given, consider greatest ukeire
    // when context given, consider tiles in the pond

    addTile(tsumo);
    std::vector<std::tuple<std::string, std::string>> vtss = getDiscard();

    // count wait
    int ucMax = 0; // max ukeire count
    int uc = 0; // ukeire count for each dahai
    int dh = 0; // dahai of max ukeire count
    for (auto it = vtss.begin(); it != vtss.end(); it++) {
        auto [d, w] = *it;
        uc = 0;
        std::vector<int> w33 = _MPSZto33(w);
        for (int i = 0; i < K; i++) {
            if (w33[i] > 0) uc = uc + 4 - arr[i]; // minus tiles already in hand
        }
        if (uc > ucMax) {
            ucMax = uc;
            // update dahai
            switch (d[1]) {
                case 'm':
                dh = d[0] - 48 - 1;
                    break;
                case 'p':
                dh = d[0] - 48 - 1 + 9;
                    break;
                case 's':
                dh = d[0] - 48 - 1 + 18;
                    break;
                case 'z':
                dh = d[0] - 48 - 1 + 27;
                    break;
            }
        }
    }

    // remove tile w highest ukeire from hand
    removeTile(dh);

    // save number of wait tiles
    numWait = ucMax;
    return dh;
}

void Hand::readMJLOG(std::string mjlog) {
    // Reads string extracted from a MJLOG log file
    // mjlog = "116,79,38,134,91,2,18,95,92,68,70,8,113"
    // Divide each number by 4 to convert them into compact encoding

    size_t i1 = 0;
    size_t i2 = mjlog.find(",", 0);
    int t;
    mjlog.append(1, ','); // so that the last number wont be left out
    while (i2 != std::string::npos && i1 != mjlog.size()) {
        t = std::atoi(mjlog.substr(i1, i2 - i1).c_str());
        addTile(t / 4);
        // update indices
        i1 = i2 + 1;
        i2 = mjlog.find(",", i1 + 1);
    }
}

void Hand::readMPSZ(std::string mpsz) {
    std::string raw = mpsz;
    // clear hand
    clear();

    char a = mpsz[0];
    if (a != 'm' && a != 'p' && a != 's' && a != 'z') {
        // reverse strings like "123m"
        // use std::reverse from <algorithm>
        std::reverse(mpsz.begin(), mpsz.end());
        a = mpsz[0];
        if (a != 'm' && a != 'p' && a != 's' && a != 'z') {
            _invalidHand(raw);
        }
    }

    int suit = 0;
    for (auto iter = mpsz.begin(); iter != mpsz.end(); iter++) {
        char a = *iter;
        switch (a) {
            case 'm':
                suit = -1;
                break;
            case 'p':
                suit = 8;
                break;
            case 's':
                suit = 17;
                break;
            case 'z':
                suit = 26;
                break;
            default:
                // in case of numbers and exceptions
                // 48--57 is range of 0 to 9 in ASCII code
                if (!(48 <= a && a <= 57)) _unknownChar(raw, a);

                // no 8z, 9z, 0z
                if (suit == 26) {
                    if (a == '8' || a == '9' || a == '0') _invalidHand(raw);
                }

                // aka dora
                if (a == '0') {
                    a += 5;
                    switch (suit) {
                        case -1:
                            // check if multiple aka dora
                            if (hasAka5m) _invalidHand(raw);
                            hasAka5m = true;
                            break;
                        case 8:
                            if (hasAka5p) _invalidHand(raw);
                            hasAka5p = true;
                            break;
                        case 17:
                            if (hasAka5s) _invalidHand(raw);
                            hasAka5s = true;
                            break;
                    }
                }

                arr[a - 48 + suit]++;
                numTile++;
        }
    }

    // go thru arr and see if there are 5 tiles of a kind (illegal)
    for (auto iter = arr.begin(); iter != arr.end(); iter++) {
        if (*iter > 4) _invalidHand(raw);
    }

    // hand is invalid if numTiles is a multiple of 3
    //if (numTile % 3 == 0) _invalidHand(raw);
}

void Hand::readTenhou(std::vector<int> th) {
    clear();

    for (int i: th) {
        int suit = i / 10;
        int tile = i % 10;
        switch (suit) {
            // 1 = manzu, 2 = pinzu, 3 = souzu, 4 = wind & sangenpai
            case 1:
                arr[tile - 1]++;
                numTile++;
                break;
            case 2:
                arr[tile - 1 + 9]++;
                numTile++;
                break;
            case 3:
                arr[tile - 1 + 18]++;
                numTile++;
                break;
            case 4:
                arr[tile - 1 + 27]++;
                numTile++;
                break;
            case 5:
                // 51, 52, 53 = 0m, 0p, 0s
                switch (tile) {
                    case 1:
                        arr[4]++;
                        hasAka5m = true;
                        break;
                    case 2:
                        arr[13]++;
                        hasAka5p = true;
                        break;
                    case 3:
                        arr[22]++;
                        hasAka5s = true;
                        break;
                }
                numTile++;
                break;
            // end case
        }
    }

    // no check done in this method!
}

void Hand::removeTile(int i33) {
    if (i33 < 0 || 33 < i33) return;
    if (arr[i33] > 0) {
        arr[i33]--;
        numTile--;
        isAnalyzed = false;
    }
}

void Hand::removeTile(int i33, int times) {
    while (0 <= i33 && i33 <= 33 && times > 0 && arr[i33] > 0) {
        arr[i33]--;
        numTile--;
        times--;
        isAnalyzed = false;
    }
    return;
}

Hand Hand::operator+(const Hand& h2) const {
    // return a hand that has tiles from both hands
    Hand h;
    for (int i = 0; i < K; i++) {
        h.arr[i] = this->arr[i] + h2.arr[i];
    }
    h.numTile = this->numTile + h2.numTile;
    h.hasAka5m = this->hasAka5m || h2.hasAka5m;
    h.hasAka5p = this->hasAka5p || h2.hasAka5p;
    h.hasAka5s = this->hasAka5s || h2.hasAka5s;

    return h;
}

Hand Hand::operator-(const Hand& h2) const {
    // return a hand whose tiles are those h1 has but h2 does not have
    Hand h;
    int numTile = 0;
    for (int i = 0; i < K; i++) {
        if (this->arr[i] >= h2.arr[i]) {
            h.arr[i] = this->arr[i] - h2.arr[i];
            numTile += h.arr[i];
        }
    }
    h.numTile = numTile;
    // h has red only if h1 has red and h2 does not have red
    h.hasAka5m = this->hasAka5m && (!h2.hasAka5m);
    h.hasAka5p = this->hasAka5p && (!h2.hasAka5p);
    h.hasAka5s = this->hasAka5s && (!h2.hasAka5s);

    return h;
}

//============================= PRIVATE METHODS ================================

std::string Hand::_33toMPSZ(const std::vector<int>& arr33) const {
    std::string mpsz;
    bool currSuit = false; // flag set if hand has tile in current suit
    bool printSuit = false;
    char suit;

    int i = 0;
    for (auto iter = arr33.cbegin(); iter != arr33.cend(); iter++, i++) {
        if (*iter > 0) {
            if ((i == 4 && hasAka5m) || (i == 13 && hasAka5p) || (i == 22 && hasAka5s)) {
                mpsz.append(1u, '0');
                mpsz.append(*iter-1, '5');
            }
            else {
                mpsz.append(*iter, i % 9 + 1 + 48);
            }
            currSuit = true;
        }

        printSuit = true;
        switch (i) {
            case 8:
                suit = 'm';
                break;
            case 17:
                suit = 'p';
                break;
            case 26:
                suit = 's';
                break;
            case 33:
                suit = 'z';
                break;
            default:
                printSuit = false;
        }

        if (printSuit && currSuit) {
            mpsz.append(1u, suit);
            currSuit = false;
        }
    }
    return mpsz;
}

std::string Hand::_33toMPSZ(const int& i33) const {
    std::string result = "";
    result.append(1u, i33 % 9 + 1 + 48);
    switch(i33 / 9) {
        case 0:
            result.append(1u, 'm');
            break;
        case 1:
            result.append(1u, 'p');
            break;
        case 2:
            result.append(1u, 's');
            break;
        case 3:
            result.append(1u, 'z');
            break;
    }
    return result;
}

void Hand::_analyze() {
    auto [s, m, d, w] = csht(arr, numTile / 3, 7);
    shanten = s - 1;
    isChiitoi = (m == 2);

    std::fill(discard.begin(), discard.end(), 0);
    int i = 0;
    while (d != 0) {
        discard[i] = d % 2;
        d = d / 2;
        i++;
    }

    std::fill(wait.begin(), wait.end(), 0);
    i = 0;
    while (w != 0) {
        wait[i] = w % 2;
        w = w / 2;
        i++;
    }

    isAnalyzed = true;
}

void Hand::_makeMax() {
    // make non-zero values in arr 4
    for (int i = 0; i < K; i++) {
        if (arr[i] != 0) {
            arr[i] = 4;
        }
    }
}

std::vector<int> Hand::_MPSZto33(std::string mpsz) const {
    // does not support aka dora!
    // assumes mpsz is valid hand!
    std::vector<int> arr33(K, 0);

    std::reverse(mpsz.begin(), mpsz.end());

    int suit = 0;
    for (size_t i = 0; i < mpsz.size(); i++) {
        char a = mpsz[i];
        switch (a) {
            case 'm':
                suit = -1;
                break;
            case 'p':
                suit = 8;
                break;
            case 's':
                suit = 17;
                break;
            case 'z':
                suit = 26;
                break;
            default:
                // in case of numbers
                arr33[a - 48 + suit]++;
        }
    }
    return arr33;
}


CalshtDW Hand::csht;
