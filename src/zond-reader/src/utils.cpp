#include <util.h>
#include <algorithm>


std::string
str_toLower(const std::string& str) {
	auto s(str);
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); }
                  );
    return s;
}

std::string
str_toUpper(const std::string& str) {
	auto s(str);
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::toupper(c); }
                  );
    return s;
}
