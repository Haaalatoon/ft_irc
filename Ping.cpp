#include "Server.hpp"

void Server::handlePing(int fd, std::vector<std::string> args) {
    std::string token = args.empty() ? "" : args[0];
    std::string response = ":" + serverName + " PONG " + serverName + " :" + token + "\r\n";
    sendRaw(fd, response);
}