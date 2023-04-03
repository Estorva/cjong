#include "kyoku.hpp"

// Paifu: https://mahjongsoul.game.yo-star.com/?paipu=<string>
// Structure of one kyoku in JSON file:
// "log": [ [ [ 0, 0, 0 ],                                     <- [kyoku (0=ton, 1=kyoku), honba, kyoutaku]
//     [ 25000, 25000, 25000, 25000 ],                         <- [initial points of self, shimo, toimen, and kami]
//     [ 32 ],                                                 <- dora indicator
//     [],                                                     <- ura dora indicator
//     [ 11, 14, 17, 26, 29, 31, 32, 33, 34, 35, 35, 39, 46 ], <- self hapai
//     [ 47, 19, 11, 19, 18, 14, 42, 29, 42, 15, 12 ],         <- self draw
//     [ 46, 47, 39, 29, 19, 35, 60, 60, 60, 26, 60 ],         <- self discard
//     [ 15, 16, 17, 19, 23, 26, 27, 33, 34, 36, 46, 46, 47 ], <- shimo haipai
//     [ "p464646", 43, 41, 53, 25, 38, 44, 44, 43, 13, 21 ],  <- shimo draw (including pon from left of shimo)
//     [ 47, 60, 60, 19, 23, 60, 60, 60, 60, 60, 60 ],
//     [ 12, 16, 17, 18, 21, 22, 22, 24, 33, 36, 37, 43, 45 ],
//     [ 27, 17, 41, 34, 23, "c383637", 34, 27, 29, 23, "2222p22", 32 ],
//     [ 21, 43, 60, 45, 17, 27, 60, 60, 12, 29, 24 ],
//     [ 14, 22, 23, 52, 26, 28, 35, 36, 37, 38, 44, 45, 47 ],
//     [ 13, 15, 47, 42, 39, 28, 37, 43, 33, 18, 41 ],
//     [ 47, 28, 60, 60, 45, 60, 44, 60, 23, 22, 60 ],
//     [ "和了", [ -1000, -500, 2000, -500 ],                  <- win condition, point difference
//     [ 2, 2, 2, "30符2飜500-1000点", "All Simples(1飜)", "Dora(1飜)" ] ] ] <- who won, points from (self if tsumo), who won or if pao: who's responsible ,fu and han and point, list of yaku's
//     ...]

// Static objects

Seat Seat::EAST(0);
Seat Seat::SOUTH(1);
Seat Seat::WEST(2);
Seat Seat::NORTH(3);

// Functions

void Kyoku::analyze(Seat seat) {
    std::cout << "========== " << getTitle() << " ==========\n";
    // reverse order of sa
    std::stack<Action> _sa = sa, sb;
    while (!_sa.empty()) {
        // reverse order of sa
        sb.push(_sa.top());
        _sa.pop();
    }

    // add first dora indicator to pond
    pond.clear();
    pond.addTile(doraInd.front());
    doraInd.pop();

    // flag of if any players are in riichi
    bool inRiichi = false;

    // The way MJS counts turn is how many times you have drawn a tile
    // or made any calls.
    int turn = 0;
    while (!sb.empty()) {
        Action a = sb.top();
        sb.pop();
        int agent = (a.agent)();

        if (a.naki == TSUMO) {
            std::cout << "Game ends in tsumo.\n";
            continue;
        }

        // update tehai
        switch (a.naki) {
            // Dont add claimed tiles to furo - they are already included as
            // part of pond; otherwise the claimed tile will be counted twice
            case CHI:
                hands[agent].removeTile(a.chi1);
                hands[agent].removeTile(a.chi2);
                hands[agent+4].addTile(a.chi1);
                hands[agent+4].addTile(a.chi2);
                break;
            case PON:
                hands[agent].removeTile(a.draw, 2);
                hands[agent+4].addTile(a.draw, 2);
                break;
            case ANKAN:
                hands[agent].removeTile(a.draw, 3);
                hands[agent+4].addTile(a.draw, 4);
                break;
            case MINKAN:
                hands[agent].removeTile(a.draw, 3);
                hands[agent+4].addTile(a.draw, 3);
                break;
            case KAKAN:
                hands[agent+4].addTile(a.draw);
                break;
            default:
                hands[agent].addTile(a.draw);
                break;
        }

        // output
        if (a.agent == seat && !inRiichi) {
            // update turn
            turn++;

            // output analysis of self
            std::cout << "Turn " << std::left << turn << ", "
                      << a.getStringNaki() << " " << cjong::_33toMPSZ(a.draw) << '\n';

            if (a.naki == TSUMO || a.naki == KAKAN || a.naki == ANKAN || a.naki == MINKAN) continue;

            std::cout << "Tehai: " << hands[agent].getMPSZ() << ", "
                      << hands[agent].getStringShanten() << ".\n";

            Hand vis = hands[4] + hands[5] + hands[6] + hands[7] + pond; // furo + kawa
            auto vtssi = hands[agent].getDiscard(vis);

            // get index of player's discard choice in returned discard list `vtssi'
            size_t index = 0;
            for (; index < vtssi.size(); index++) {
                auto [cut, wait, left] = vtssi[index];
                if (cjong::MPSZto33(cut) == a.discard) break;
            }

            // get maximum width of cut choices
            size_t maxWidth = 5;
            for (size_t i = 0; i < vtssi.size(); i++) {
                size_t w = std::get<1>(vtssi[i]).size() + 1;
                maxWidth = (w > maxWidth) ? w : maxWidth;
            }
            int leftMax = std::get<2>(vtssi[0]);

            std::cout << std::left;
            std::cout << "   Cut " << std::setw(maxWidth) << "Wait" << std::setw(3) << "#" << "\n";
            std::cout << "-------" << std::string(maxWidth, '-') << "---\n";

            for (size_t i = 0; i < vtssi.size(); i++) {
                auto [cut, wait, left] = vtssi[i];
                // print arrow indicating player's choice, cut, number of tiles left
                // and color the player's choice
                if (i == index) {
                    // if choice has max wait, print green, otherwise print yellow
                    if (left == leftMax) std::cout << "\033[1;32m-> ";
                    else std::cout << "\033[1;33m-> ";
                }
                else std::cout << "   ";
                std::cout << std::setw(4) << cut << std::setw(maxWidth) << wait
                          << std::setw(3) << left << "\033[0m\n";
            }

            // if player's discard choice is not in discard list, print red
            if (index == vtssi.size()) {
                std::cout << "\033[1;31m-> " << cjong::_33toMPSZ(a.discard) << "\033[0m\n";
            }

            std::cout << "\n";
            if (a.naki == RIICHI) {
                std::cout << "Player riichis with " << cjong::_33toMPSZ(a.discard) << "\n";
                inRiichi = true;
            }
        }
        else if (a.naki != NO_NAKI) {
            std::cout << "(" << a.agent.getString() << " " << a.getStringNaki() << " ";
            switch (a.naki) {
                case CHI:
                case PON:
                case ANKAN:
                case KAKAN:
                case MINKAN:
                case TSUMO:
                    std::cout << cjong::_33toMPSZ(a.draw);
                    break;
                case RIICHI:
                    std::cout << cjong::_33toMPSZ(a.discard);
                    inRiichi = true;
                    break;
                case NO_NAKI:
                    // dummy case to suppress compiler warning
                    break;
            }
            std::cout << ")\n";
        }

        // reveal dora indicator
        if ((a.naki == ANKAN || a.naki == KAKAN || a.naki == MINKAN) && !doraInd.empty()) {
            pond.addTile(doraInd.front());
            doraInd.pop();
        }

        // discard
        if (a.naki == TSUMO || a.naki == ANKAN || a.naki == KAKAN || a.naki == MINKAN) continue;
        hands[agent].removeTile(a.discard);
        pond.addTile(a.discard);
    }
}

std::string Kyoku::getTitle() {
    std::string kyokuWind;
    switch (kw) {
        case 0:
            kyokuWind = "East";
            break;
        case 1:
            kyokuWind = "South";
            break;
        case 2:
            kyokuWind = "West";
            break;
    }
    std::string res = kyokuWind + " " + std::to_string(kn) + ", " + std::to_string(hb) + " repeat";
    return res;
}

void Kyoku::readJSON(json raw) {
    // Contents of input JSON object:
    // raw[0]: vector<int>(3),
    //     - kyoku
    //     - honba
    //     - kyoutaku
    // raw[1]: vector<int>(4),       initial points of players
    // raw[2]: vector<string>(*),    dora indicator
    // raw[3]: vector<string>(*),    uradora indicator, present if someone declares riichi and wins
    // raw[4-7]: vector<string>(13), haipai (chicha, shimo, toimen, kami)
    // raw[8]:                       array of actions in custom structure
    // raw[9]: vector<*>(*),   end condition
    //     - raw[9] = vector<*>(3) when a player tsumo or ron,
    //         + "和了"
    //         + payment
    //         + players involved in payment and list of yaku
    //     - raw[9] = vector<*>(2) when ryukyoku
    //         + "Ryuukyoku"
    //         + payment, if any
    // * means uncertain number or type
    // game always starts with self = whoever sits at east (chicha)

    // read kyoku info
    kw = raw[0][0].get<int>() / 4;
    kn = raw[0][0].get<int>() % 4 + 1;
    hb = raw[0][1].get<int>();

    // read dora indicator
    auto di = raw[2].get<std::vector<std::string>>();
    for (auto i : di) doraInd.push(cjong::MPSZto33(i));

    Action a;
    // Seat agent;   // the player who makes this call
    // Seat patient; // the player whose discard triggered this call
    // Naki naki;
    // int draw;
    // int discard;
    // int chi1; // tile used in chii
    // int chi2; // tile used in chii

    for (size_t i = 0; i < 4; i++) {
        for (size_t j = 0; j < 13; j++) {
            hands[0+i].addTile(cjong::MPSZto33(raw[4+i][j].get<std::string>()));
        }
        if (raw[4+i].size() == 14) {
            // the dealer is given an extra tile at haipai
            // make this 14th tile into part of an Action object
            a.draw = cjong::MPSZto33(raw[4+i][13].get<std::string>());
        }
    }

    //////////////////////////////////////////////////
    // 03/22: rework of parsing JSON with updated downloader script
    // raw[8] = [
    //     { "type": "T", "who": 0, "tile": "1m" },
    //     { "type": "D", "who": 0, "tile": "3z" },
    //     { "type": "T", "who": 1, "tile": "9m" },
    //     { "type": "D", "who": 1, "tile": "1m" },
    //     { "type": "T", "who": 2, "tile": "4p" },
    //     { "type": "D", "who": 2, "tile": "1p" }, ...
    // ]
    // type - Type of action
    //     T - tsumo
    //     D - dahai
    //     A - ankan
    //     K - kakan
    //     M - minkan
    //     P - pon
    //     C - chi
    //     R - riichi
    // who  - The player who performed this action
    // tile - The tile involved in this action
    // furo - The tiles that are revealed

    for (size_t i = 0; i < raw[8].size(); i++) {
        int who = raw[8][i]["who"].get<int>();
        a.agent = Seat(who);

        std::string tileMPSZ = raw[8][i]["tile"].get<std::string>();
        int t = cjong::MPSZto33(tileMPSZ);
        int f1 = -1;
        int f2 = -1;

        // switch quantity cannot be string
        std::string type = raw[8][i]["type"].get<std::string>();
        int n = (type == "T" ? 0 :
                (type == "D" ? 1 :
                (type == "A" ? 2 :
                (type == "K" ? 3 :
                (type == "M" ? 4 :
                (type == "P" ? 5 :
                (type == "C" ? 6 :
                (type == "R" ? 7 : -1
        ))))))));

        switch (n) {
            case 0: // Draw
                a.draw = t;
                break;
            case 1: // Discard
                a.discard = t;
                a.naki = NO_NAKI;
                break;
            case 2: // Ankan
                a.draw = t;
                a.naki = ANKAN;
                break;
            case 3: // Kakan
                a.draw = t;
                a.naki = KAKAN;
                break;
            case 4: // Minkan
                a.draw = t;
                a.naki = MINKAN;
                break;
            case 5: // Pon
                a.draw = t;
                a.naki = PON;
                break;
            case 6: // Chi
                a.draw = t;
                a.naki = CHI;
                f1 = cjong::MPSZto33(raw[8][i]["furo"][0].get<std::string>());
                f2 = cjong::MPSZto33(raw[8][i]["furo"][1].get<std::string>());
                break;
            case 7: // Riichi
                a.discard = t;
                a.naki = RIICHI;
                break;
            default:
                std::cout << "Something is wrong." << std::endl;
                exit(1);
        }

        // Conclude an action object `a' in these cases:
        // 1. Discard
        // 2. Ankan
        // 3. Kakan
        // 4. Riichi
        // 5. Minkan
        if (type == "D" || type == "A" || type == "K" || type == "R" || type == "M") {
            sa.push(a);
            a.naki = NO_NAKI;
        }
    }
}

void Kyoku::readMJLOG(std::string raw) {
    // https://m77.hatenablog.com/entry/2017/05/21/214529
    // Content of input string:
    // <INIT
    //     seed="0,0,0,5,1,5"
    //     ten="250,250,250,250"
    //     oya="0"
    //     hai0="4,58,0,91,23,47,20,105,133,119,86,46,135"
    //     hai1="54,33,10,9,12,67,35,117,18,115,134,77,120"
    //     hai2="56,40,82,22,130,42,104,45,19,110,29,132,55"
    //     hai3="112,70,100,43,14,97,92,75,1,68,98,30,48" />
    // <T80/>
    // <D119/>
    // ...
    // <N who="0" m="50762" />
    // ...
    // <DORA hai="8" />
    // ...
    // <REACH who="3" step="1"/>
    // <G98/>
    // <REACH who="3" ten="250,250,250,240" step="2"/>
    // ...
    // <RYUUKYOKU
    //     ba="0,1"
    //     sc="250,10,250,10,250,-30,240,10"
    //     hai0="0,4,20,26,28,85,86,87,91,93"
    //     hai1="9,10,11,12,13,18,21,24,54,57"
    //     hai3="8,14,15,16,17,68,70,92,95,97,99,100,101" />
    //
    // Last XML element could also be:
    // <AGARI
    //     ba="1,1"
    //     hai="0,2,10,11,53,55,69,71,80,82,88,91,105,106"
    //     machi="106"
    //     ten="25,3200,0"
    //     yaku="22,2,54,1"
    //     doraHai="27"
    //     who="2"
    //     fromWho="3"
    //     sc="297,0,463,0,122,45,108,-35"
    //     owari="297,9.7,463,56.3,167,-23.3,73,-42.7" />
    //
    // INIT
    //     seed: kyoku, honba, number of riichi stick, dice 1, dice 2, dora indicator
    //     ten: points divided by 100
    //     oya: dealer (relative to chicha)
    //     hai0 - hai3: haipai
    // T80
    //     First letter indicates the player who draws a tile (T: 0, U: 1, V: 2, W: 3)
    //     The following number indicates the drawn tile (see later)
    // D119
    //     First letter indicates the player who discards a tile (D: 0, E: 1, F: 2, G: 3)
    //     The following number indicates the drawn tile
    // N
    //     who: the player who makes a naki
    //     m: meld format (see later)
    // DORA
    //     hai: the new dora indicator
    // REACH
    //     who: the player who makes a naki
    //     step: 1 means riichi declaration, 2 means the riichi is valid
    //     ten: player points after the riichi declaration
    // RYUUKYOKU
    //     ba: honba stick and riichi stick
    //     sc: pairs of (player point before tenpai payment) and (tenpai payment)
    //     hai0 - hai3: tehai of players in tenpai
    // AGARI
    //     ba: honba stick and riichi stick
    //     hai: completed hand of the winner
    //     m: furo, if any
    //     machi: the tile that completes the winner
    //     ten: fu, point, rank (mangan, haneman, baiman, sanbaiman, yakuman, or non of the above)
    //     yaku: pairs of yaku and their han number
    //     doraHai: dora indicator
    //     doraHaiUra: uradora indicator, if any
    //     who: winner
    //     fromWho: the player who dealt in
    //     sc: player points and payment
    //     owari: final points and PT (including uma)

    // read kyoku info
    int k = std::stoi(cjong::fromIndexToChar(raw, raw.find('"') + 1, ',').c_str());
    kw = k / 4;
    kn = k % 4 + 1;
    hb = std::stoi(cjong::afterIndexBetweenChar(raw, raw.find('"'), ',').c_str());

    // read haipai
    std::string hp0 = cjong::fromIndexToChar(raw, raw.find('"', raw.find("hai0")) + 1, '"');
    std::string hp1 = cjong::fromIndexToChar(raw, raw.find('"', raw.find("hai1")) + 1, '"');
    std::string hp2 = cjong::fromIndexToChar(raw, raw.find('"', raw.find("hai2")) + 1, '"');
    std::string hp3 = cjong::fromIndexToChar(raw, raw.find('"', raw.find("hai3")) + 1, '"');
    hands[0].readMJLOG(hp0);
    hands[1].readMJLOG(hp1);
    hands[2].readMJLOG(hp2);
    hands[3].readMJLOG(hp3);

    std::size_t idx = 1; // first element after <INIT .../>
    Action a;
    bool isRiichi = false;
    while ((idx = raw.find('<', idx+1)) != std::string::npos) {
        char c = raw[idx + 1];
        char d = raw[idx + 2];

        switch (c) {
            // draw
            case 'T':
                a.agent = Seat::EAST;
                break;
            case 'U':
                a.agent = Seat::SOUTH;
                break;
            case 'V':
                a.agent = Seat::WEST;
                break;
            case 'W':
                a.agent = Seat::NORTH;
                break;
            // reveal new dora indicator
            case 'D':
                if (d == 'O') {
                    std::string sdi = cjong::afterIndexBetweenChar(raw, idx, '"');
                    doraInd.push(std::atoi(sdi.c_str()) / 4);
                }
                break;
            // naki
            case 'N': {
                // Wrap this part of code in {} since we have declared a local
                // variable `m'; otherwise C++ compiler complains "error: jump
                // to case label".
                a.agent = Seat(std::atoi(cjong::afterIndexBetweenChar(raw, idx, '"').c_str()));
                int m = std::atoi(cjong::afterIndexBetweenChar(raw, idx + 11, '"').c_str());

                if (m & 0x4) {
                    // chi
                    // e.g. in case of 34s chi 2s, m == 46159 (0xB44F), m & 0x4 == 1 (flag)
                    // (m & 0xfc00) >> 10 == 45 (0b101101),
                    // 45 / 3 = 15 == 15th meld of all possible melds
                    // (0th = 123m, 1st = 234m, ..., 20th = 789p)
                    // 45 % 3 = 0 == the claimed tile is the lowest tile
                    // (1 = kanchan, 2 = higher end of ryanmen)
                    a.naki = CHI;
                    int n = m >> 10;
                    int p = n / 3;
                    int q = n % 3;
                    int l; // lowest tile in compact encoding

                    switch (p / 7) {
                        case 0:
                            // manzu sequence
                            l = p;
                            break;
                        case 1:
                            // souzu sequence
                            l = p % 7 + 9;
                            break;
                        case 2:
                            // pinzu sequence
                            l = p % 7 + 18;
                            break;
                    }

                    switch (q) {
                        case 0:
                            a.draw = l;
                            a.chi1 = l+1;
                            a.chi2 = l+2;
                            break;
                        case 1:
                            a.chi1 = l;
                            a.draw = l+1;
                            a.chi2 = l+2;
                            break;
                        case 2:
                            a.chi1 = l;
                            a.chi2 = l+1;
                            a.draw = l+2;
                            break;
                    }
                }
                else if (m & 0x18) {
                    // pon or kakan
                    // e.g. in case of hatsu pon, m == 50186 (0xC40A), m & 0x8 = 1 (flag)
                    // (m & 0xFE00) >> 9 == 98 (0b1100010)
                    // 98 / 3 = 32, which is hatsu in 33 compact encoding
                    if (m & 0x8) a.naki = PON;
                    if (m & 0x10) a.naki = KAKAN;
                    a.draw = (m >> 9) / 3;
                }
                else {
                    // minkan or ankan
                    if ((m & 0x3) == 0) a.naki = ANKAN;
                    else a.naki = MINKAN;
                    a.draw = (m >> 8) / 4;
                }
                break;
            }
            // riichi
            // game end
            case 'R':
                if (d != 'Y') {
                    // riichi
                    // Riichi element appears twice for a valid riichi declaration
                    // A discard tile lies between these two elements
                    // Toggle flag `isRiichi' and parse the discard tile accordingly
                    isRiichi = !isRiichi;
                    if (isRiichi) a.naki = RIICHI;
                }
                break;
        }

        // c == 'T', 'U', 'V', or 'W'
        if ('T' <= c && c <= 'W')
            a.draw = std::atoi(cjong::fromIndexToChar(raw, idx+2, '/').c_str()) / 4;

        // c == 'D', 'E', 'F', or 'G'
        if ('D' <= c && c <= 'G' && d != 'O')
            a.discard = std::atoi(cjong::fromIndexToChar(raw, idx+2, '/').c_str()) / 4;

        // Conclude an action object `a' in these cases:
        // 1. <T.../><D.../>
        // 2. <T.../><N.../> if N is ankan or kakan
        // 2. <T.../><REACH who="0" step="1"/><D.../>
        // 3. <N.../><D.../> if N is chi or pon
        // 4. <N.../> if N is minkan

        if (('D' <= c && c <= 'G' && d != 'O') || (c == 'N' && a.naki == MINKAN)) {
            //std::cout << "PUSH " << a.agent << " " << a.naki << " " << a.draw << " " << a.discard << "\n";
            sa.push(a);
            a.naki = NO_NAKI;
        }
    }
}

void Kyoku::replay() {
    std::string kyokuWind;
    switch (kw) {
        case 0:
            kyokuWind = "East";
            break;
        case 1:
            kyokuWind = "South";
            break;
        case 2:
            kyokuWind = "West";
            break;
    }
    std::cout << "===== " << kyokuWind << " " << kn << ", " << hb << " repeat =====\n";

    std::stack<Action> _sa = sa, sb;
    while (!_sa.empty()) {
        // reverse order of sa
        sb.push(_sa.top());
        _sa.pop();
    }
    while (!sb.empty()) {
        Action a = sb.top();
        sb.pop();
        std::string verb;
        std::string verb2;
        std::cout << std::setw(6) << std::left << a.agent.getString();

        switch (a.naki) {
            case ANKAN:
                verb = "ankans";
                break;
            case KAKAN:
                verb = "kakans";
                break;
            case RIICHI:
                verb = "draws";
                verb2 = "and riichis";
                break;
            case TSUMO:
                verb = "draws";
                verb2 = "and wins.";
                break;
            case NO_NAKI:
                verb = "draws";
                verb2 = "and cuts";
                break;
            case CHI:
                verb = "chiis";
                verb2 = "and cuts";
                break;
            case PON:
                verb = "pons";
                verb2 = "and cuts";
                break;
            case MINKAN:
                verb = "kans";
                break;
        }
        std::cout << std::setw(7) << std::left << verb;
        std::cout << std::setw(3) << a.draw;

        if (a.naki == RIICHI || a.naki == TSUMO || a.naki == NO_NAKI || a.naki == CHI || a.naki == PON)
            std::cout << std::setw(12) << std::left << verb2 << std::setw(3) << a.discard;

        std::cout << '\n';
    }
}
