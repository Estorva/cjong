#include "game.hpp"

void Game::analyze() {
    Seat s = JICHA;
    if (vn[0] == j) s = JICHA;
    if (vn[1] == j) s = SHIMO;
    if (vn[2] == j) s = TOIMEN;
    if (vn[3] == j) s = KAMI;

    std::cout << "=============== Game Info ===============" << "\n";
    std::cout << "East  " << vn[0] << " " << ((s == JICHA) ? "(*)" : " ") << "\n";
    std::cout << "South " << vn[1] << " " << ((s == SHIMO) ? "(*)" : " ") << "\n";
    std::cout << "West  " << vn[2] << " " << ((s == TOIMEN) ? "(*)" : " ") << "\n";
    std::cout << "North " << vn[3] << " " << ((s == KAMI) ? "(*)" : " ") << "\n";
    std::cout << "\n";

    for (size_t i = 0; i < vk.size(); i++) {
        Kyoku* k = vk[i];
        k->analyze(s);
    }
}

void Game::analyze(size_t i) {
    if (i >= vk.size()) return;
    vk[i]->analyze(JICHA);
}

void Game::initialize() {
    while (!vk.empty()) {
        Kyoku* k = vk[0];
        delete k;
        vk.erase(vk.begin());
    }
    vn[0] = vn[1] = vn[2] = vn[3] = "";
}

void Game::read(std::string file) {
    // determine which read function to call depending on the first character of file
    std::ifstream fstr(file);
    std::string raw;
    std::getline(fstr, raw);

    if (raw[0] == '{') readJSON(file);
    else if (raw[0] == '<') readMJLOG(file);
    else std::cout << "Error: unknown file format!\n";
}

void Game::readJSON(std::string file) {
    // Reads Tenhuo 6 JSON format given file name
    std::ifstream fstr(file);
    json j = json::parse(fstr);
    json jlog = j["log"];

    for (auto it = jlog.begin(); it != jlog.end(); it++) {
        Kyoku* k = new Kyoku();
        k->readJSON(*it);
        vk.push_back(k);
    }

    json jname = j["name"];
    for (int i = 0; i < 4; i++) vn[i] = jname[i].get<std::string>();
}

void Game::readMJLOG(std::string file) {
    // Reads MJLOG format given file name
    std::ifstream fstr(file);
    std::string raw;
    std::getline(fstr, raw); // raw file is a long one-liner

    std::size_t iInit = 0;
    std::size_t iRA; // index of "RYUUKYOKU" or "AGARI"
    std::size_t iEnd; // index of '>'

    while ((iInit = raw.find("INIT", iInit + 10)) != std::string::npos) {
        // Extract substring "<INIT .../>...<RYUUKYOKU .../>" or
        // "<INIT .../>...<AGARI .../>" and pass it to Kyoku objects.
        // 1. Find substring "INIT"
        // 2. Find "RYUUKYOKU" or "AGARI" after "INIT"
        // 3. Pass the substring in between to a new Kyoku object
        // 4. Repeat until all "INIT" substrings are found

        iInit -= 1;  // start at '<'

        // set iRA to whichever comes first
        iRA = std::min(raw.find("RYUUKYOKU", iInit), raw.find("AGARI", iInit));
        iEnd = raw.find('>', iRA) + 1;
        Kyoku* k = new Kyoku();
        k->readMJLOG(raw.substr(iInit, iEnd - iInit));
        vk.push_back(k);
    }

    std::size_t iUN = raw.find("UN"); // index of <UN ...>
    std::size_t iN = raw.find("n", iUN);
    int i = 0;
    while (iN != std::string::npos && i < 4) {
        vn[i++] = cjong::decodeUTF8(cjong::afterIndexBetweenChar(raw, iN, '\"'));
        iN = raw.find("n", iN + 1);
    }
}

void Game::replay(int i) {
    vk[i]->replay();
}

/******************************* MJLOG Format **********************************
raw URL: https://tenhou.net/0/?log=2023020601gm-0009-0000-33fbd68e
log URL: https://tenhou.net/0/log/?2023020601gm-0009-0000-33fbd68e
raw URL may come with string "&tw=1" which specifies player view
(0=chicha=ton, 1=nan, etc)

<mjloggm ver="2.3">
    <SHUFFLE
        seed="..."
        ref="" />
    <GO
        type="9"
        lobby="0" />
    <UN
        n0="%49%64%6F%63%68%69%7A%75"
        n1="%73%70%61%67%68%65%74"
        n2="%E3%83%A2%E3%82%B8%E3%83%BC"
        n3="%E3%81%95%E3%81%A7%E3%81%83%E3%81%A1%E3%81%87%E3%82%8A"
        dan="8,10,11,3"
        rate="1281.84,1617.92,1758.59,1584.56"
        sx="M,M,M,M"/>
    <TAIKYOKU oya="0"/>
    <INIT
        seed="0,0,0,5,1,5"
        ten="250,250,250,250"
        oya="0"
        hai0="4,58,0,91,23,47,20,105,133,119,86,46,135"
        hai1="54,33,10,9,12,67,35,117,18,115,134,77,120"
        hai2="56,40,82,22,130,42,104,45,19,110,29,132,55"
        hai3="112,70,100,43,14,97,92,75,1,68,98,30,48" />
    <T80/>
    <D119/>
    ...
    <N who="0" m="50762" />
    ...
    <REACH who="3" step="1"/>
    <G98/>
    <REACH who="3" ten="250,250,250,240" step="2"/>
    ...
    <RYUUKYOKU
        ba="0,1"
        sc="250,10,250,10,250,-30,240,10"
        hai0="0,4,20,26,28,85,86,87,91,93"
        hai1="9,10,11,12,13,18,21,24,54,57"
        hai3="8,14,15,16,17,68,70,92,95,97,99,100,101" />
    <INIT .../>
    ...
    <AGARI
        ba="1,1"
        hai="14,18,20,24,26,27,44,50,52,60,62,88,93,97"
        machi="20"
        ten="30,11700,0"
        yaku="0,1,8,1,54,2"
        doraHai="110"
        who="0"
        fromWho="0"
        sc="260,130,260,-40,220,-40,250,-40" />
    </mjloggm>
</mjloggm>
*******************************************************************************/
