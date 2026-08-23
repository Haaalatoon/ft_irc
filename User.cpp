#include "Server.hpp"

void Server::handleUser(int fd, std::vector<std::string> args) {
    if (!clients[fd].getisAuthenticated()) {
        sendToClient(fd, " 451 " + (clients[fd].getNickname().empty() ? "*" : clients[fd].getNickname()) + " :You have not registered\r\n");
        return;
    }
    if (clients[fd].registered()) {
        sendToClient(fd, " 462 " + (clients[fd].getNickname().empty() ? "*" : clients[fd].getNickname()) + " :You may not reregister\r\n");
        return;
    }
    if (args.size() < 4) {
        sendToClient(fd, " 461 " + (clients[fd].getNickname().empty() ? "*" : clients[fd].getNickname()) + " :Not enough parameters\r\n");
        return;
    }
    clients[fd].setUsername(args[0]);
    clients[fd].setRealname(args[3]);
    if (clients[fd].registered())
        sendToClient(fd, " 001 " + clients[fd].getNickname() + " :Welcome to the IRC server " +
                     clients[fd].getNickname() + "!" + clients[fd].getUsername() + "@localhost\r\n");
}