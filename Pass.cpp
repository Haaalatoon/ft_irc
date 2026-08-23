#include "Server.hpp"

void Server::handlePass(int fd, std::vector<std::string> args) {
    if (clients[fd].getisAuthenticated()) {
        sendToClient(fd, " 462 * :You may not reregister\r\n");
        return;
    }
    if (args.empty() || args[0].empty()) {
        std::string nick = clients[fd].getNickname().empty() ? "*" : clients[fd].getNickname();
        sendToClient(fd, " 461 " + nick + " PASS :Not enough parameters\r\n");
        return;
    }
    if (password != args[0]) {
        sendToClient(fd, " 464 * :Password incorrect\r\n");
        return;
    }
    clients[fd].setisAuthenticated(true);
}