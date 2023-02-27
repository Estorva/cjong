#include "conversion.hpp"

// functions for conversion between MPSZ, Tenhou, and compact encoding
// MPSZ    1m 2m 3m ... 9m 1p 2p ... 1s ... 1z ... 7z 0m  0p  0s
// Tenhou  11 12 13 ... 19 21 22 ... 31 ... 41 ... 47 51  52  53
// Compact 0  1  2  ... 8  9  10 ... 18 ... 27 ... 33 N/A N/A N/A
// Tenhou 60 = tsumogiri, Tenhou 0 = no discard

int cjong::MPSZto33 (std::string mpsz) {
    // converts a single tile in MPSZ format to compact encoding

    int num = 0;
    // handle red five
    if (mpsz[0] == '0') num = 4;
    else                num = mpsz[0] - 48 - 1;

    int suit = 0;
    switch (mpsz[1]) {
        case 'm':
            suit = 0;
            break;
        case 'p':
            suit = 9;
            break;
        case 's':
            suit = 18;
            break;
        case 'z':
            suit = 27;
            break;
    }

    return suit + num;
}

std::vector<int> cjong::MPSZto33Arr (std::string mpsz) {
    // converts a string in MPSZ format to compact encoding
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

int cjong::TenhouTo33 (int iTh) {
    // converts a single integer from tenhou encoding to compact encoding
    // handle red five
    if (iTh == 51) iTh = 15;
    if (iTh == 52) iTh = 25;
    if (iTh == 53) iTh = 35;
    int suit = iTh / 10 - 1;
    int num = iTh % 10 - 1;
    return suit * 9 + num;
}

std::vector<int> cjong::TenhouTo33Arr (std::vector<int> arrTh) {
    // converts an array in tenhou encoding to an array in compact encoding
    std::vector<int> arr33(K, 0);
    for (int i : arrTh) arr33[cjong::TenhouTo33(i)]++;
    return arr33;
}

std::string cjong::_33toMPSZ (int i33) {
    // converts integer (0-33) to MPSZ
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

std::string cjong::_33toMPSZ (const std::vector<int>& arr33) {
    // convert array of integers to MPSZ string
    // does not add red five! (since compact encoding ignores red five)
    std::string mpsz;
    bool currSuit = false; // flag set if hand has tile in current suit
    bool printSuit = false;
    char suit;

    int i = 0;
    for (auto iter = arr33.cbegin(); iter != arr33.cend(); iter++, i++) {
        if (*iter > 0) {
            mpsz.append(*iter, i % 9 + 1 + 48);
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

int cjong::_33toTenhou (int i33) {
    // converts a single integer (0-33) to tenhou encoding
    int suit = i33 / 9 + 1;
    int num = i33 % 9 + 1;
    return suit * 10 + num;
}

std::vector<bool> cjong::hasAka (std::string mpsz) {
    // returns if the MPSZ string contains aka dora (0m, 0s, 0p)
    std::vector<bool> vb(3, false);
    std::reverse(mpsz.begin(), mpsz.end());
    int suit = 0;
    for (char a : mpsz) {
        switch (a) {
            case 'm':
                suit = 0;
                break;
            case 'p':
                suit = 1;
                break;
            case 's':
                suit = 2;
                break;
            case '0':
                vb[suit] = true;
                break;
        }
    }
    return vb;
}

std::vector<bool> cjong::hasAka(std::vector<int> arrTh) {
    // returns if the array of tiles in tenhou encoding contains aka dora (51, 52, 53)
    std::vector<bool> vb(3, false);
    for (int i : arrTh) {
        if (i == 51) vb[0] = true;
        if (i == 52) vb[1] = true;
        if (i == 53) vb[2] = true;
    }
    return vb;
}

std::string cjong::afterIndexBetweenChar(const std::string& str, const int& index, const char& c) {
    // return a substring enclosed by two characters specified by `c' after index `index'
    size_t i1 = str.find(c, index + 1);
    size_t i2 = str.find(c, i1 + 1);
    std::string res = "";
    if (i1 == std::string::npos || i2 == std::string::npos) return res;
    return str.substr(i1 + 1, i2 - i1 - 1);
}

std::string cjong::fromIndexToChar(const std::string& str, const int& index, const char& c) {
    // return the substring from specified index to (excluding) the specified character `c'
    size_t i1 = str.find(c, index + 1);
    std::string res = "";
    if (i1 == std::string::npos) return res;
    return str.substr(index, i1 - index);
}

std::string cjong::decodeUTF8(const std::string& str) {
    // UTF-8 encodes code points into 1~4 bytes depending on the code point
    // Code point range Byte 1   Byte 2   Byte 3
    // ---------------- ------   ------   ------
    // U+0000 -- U+007F 0xxxxxxx
    // U+0080 -- U+07FF 110xxxxx 10xxxxxx
    // U+0800 -- U+FFFF 1110xxxx 10xxxxxx 10xxxxxx
    // Argument `str' comes in the form "%73%70%61%E3%82%B8%E3%83%BC"
    // "%73" starts with 0, therefore its a 1-byte character => 's'
    // "%E3%82%B8" starts with 1110 (=E), therefore its a 3-byte character
    // 0xE382B8 = 11100011 10000010 10111000
    //                        ↓ removing bits not used to encode code point
    //                0011   000010   111000 => 0011000010111000
    //                                             3   0   B   8 => 'ジ'

    // std::mbstate_t state;
    // char u8[MB_LEN_MAX];
    //
    // for (size_t i = 0; i < str.size() / 3; i++) {
    //     // i     = '%'
    //     // i + 1 = hex
    //     // i + 2 = hex
    //     uint32_t b1 = std::stol(str.substr(3 * i + 1, 2), NULL, 16);
    //     uint32_t b2, b3, c;
    //
    //     if (b1 & 0xC0 && ~b1 & 0x20) {
    //         // if first 3 bits of b1 is 110
    //         b2 = std::stol(str.substr(3 * i + 4, 2), NULL, 16) & 0x3F;
    //         c = b1 << 6 | b2;
    //         i++;
    //     }
    //     else if (b1 & 0xE0 && ~b1 & 0x10) {
    //         // if first 3 bits of b1 is 1110
    //         b2 = std::stol(str.substr(3 * i + 4, 2), NULL, 16) & 0x3F;
    //         b3 = std::stol(str.substr(3 * i + 7, 2), NULL, 16) & 0x3F;
    //         c = b1 << 12 | b2 << 6 | b3;
    //         i+=2;
    //     }
    //     else {
    //         // if first bit of b1 is 0
    //         c = b1;
    //     }
    //
    //     auto len = std::c32rtomb(u8, c, &state);
    //     res.append(u8, len);
    // }


    std::string res = "";
    CURL *curl = curl_easy_init();
    if(curl) {
      int decodelen;
      char *decoded = curl_easy_unescape(curl, str.c_str(), str.size(), &decodelen);
      if(decoded) {
        res = decoded;
        curl_free(decoded);
      }
      curl_easy_cleanup(curl);
    }

    return res;
}
















//%
