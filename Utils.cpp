#include "Utils.hpp"
#include <cstdlib>
#include <cctype>
#include <iostream>

int validatePort(const std::string& av) {
    if (av.size() > 5) return -1;
    for (size_t i = 0; i < av.size(); ++i)
        if (!std::isdigit(av[i])) return -1;
    int port = std::atoi(av.c_str());
    if (port < 1024 || port > 65535) {
        std::cout << "Error: port must be between 1024 and 65535" << std::endl;
        return -1;
    }
    return port;
}