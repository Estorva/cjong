// g++ --std=c++17 -I/opt/local/include test_json.cpp -o build/test_json

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <boost/type_index.hpp>

using json = nlohmann::json;
using namespace boost::typeindex;

int main() {
    std::ifstream f("test.json");
    json data = json::parse(f);

    // passes, outputs '"2.3"'
    std::string s = data["ver"];
    std::cout << s << '\n';

    // passes, outputs '0'
    int t = data["log"][0][0][0];
    std::cout << t << '\n';

    // passes, outputs '[0,0,0]'
    std::vector<int> v = data["log"][0][0];
    std::cout << "[" << v[0] << "," << v[1] << "," << v[2] << "]\n";

    // coercion
    // passes, outputs `'13'
    std::vector<int> w = data["log"][0][4].get<std::vector<int>>();
    std::cout << w.size() << '\n';

    // passes, outputs '11 14 17 26 29 31 32 33 34 35 35 39 46 '
    auto self_haipai = data["log"][0][4];
    for (auto tile: self_haipai) {
        std::cout << tile.get<int>() << " ";
    }
    std::cout << "\n\n";

    // data["log"][0][8] is [ "p464646", 43, 41, 53, 25, 38, 44, 44, 43, 13, 21 ]
    auto shimo_draw = data["log"][0][8];
    for (auto tile: shimo_draw) {
        //std::cout << type_id_with_cvr<decltype(tile)>().pretty_name() << "\n";
        // each type of tile is
        // nlohmann::basic_json<std::map, std::vector, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, bool, long long, unsigned long long, double, std::allocator, nlohmann::adl_serializer, std::vector<unsigned char, std::allocator<unsigned char> > >

        // passes, outputs `3 6 6 6 6 6 6 6 6 6 6'
        // 3 = string, 6 = number_unsigned
        // declared in enum class nlohmann::basic_json::value_t
        // to compare:
        // (j.type() == json::value_t::null)
        std::cout << (int)tile.type() << " ";
    }
    std::cout << '\n';

    for (auto tile: shimo_draw) {
        // passes, outputs `p464646 43 41 53 25 38 44 44 43 13 21'
        if (tile.is_string()) {
            std::cout << tile.get<std::string>() << " ";
        }
        if (tile.is_number()) {
            std::cout << tile.get<int>() << " ";
        }
    }
    std::cout << '\n';

    auto jname = data["name"];
    std::cout << jname[0].get<std::string>() << "\n";

    return 0;
}
