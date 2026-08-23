#include "Server.hpp"
#include "Utils.hpp"
#include <iostream>
#include <cstdlib>

int main(int ac, char** av) {
    if (ac != 3) {
        std::cout << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }
    int port = validatePort(av[1]);
    if (port == -1) return 1;
    try {
        Server s(port, av[2]);
        s.start();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}