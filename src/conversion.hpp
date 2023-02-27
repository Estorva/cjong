#ifndef CONVERSION_HPP
#define CONVERSION_HPP

#include <algorithm>
#include <curl/curl.h> // curl_easy_unescape
#include <stdint.h>    // uint32_t
#include <string>      // std::stol
#include <vector>

#include "constant.hpp"

namespace cjong {
    int               MPSZto33 (std::string);
    std::vector<int>  MPSZto33Arr (std::string);

    int               TenhouTo33 (int);
    std::vector<int>  TenhouTo33Arr (std::vector<int>);

    std::string       _33toMPSZ (int);
    std::string       _33toMPSZ (const std::vector<int>&);

    int               _33toTenhou (int);

    std::vector<bool> hasAka(std::string);
    std::vector<bool> hasAka(std::vector<int>);

    std::string       afterIndexBetweenChar(const std::string&, const int&, const char&);
    std::string       fromIndexToChar(const std::string&, const int&, const char&);

    std::string       decodeUTF8(const std::string&);
}

#endif
