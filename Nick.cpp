#include "Server.hpp"
#include <cctype>

void Server::handleNick(int fd, std::vector<std::string> args) {
    if (!clients[fd].getisAuthenticated()) {
        sendToClient(fd, " 451 " + (clients[fd].getNickname().empty() ? "*" : clients[fd].getNickname()) + " :You have not registered\r\n");
        return ;
    }
    
    if (args.empty() || args[0].empty()) {
        sendToClient(fd, " 431 * :No nickname given\r\n");
        return ;
    }

    std::string nick = args[0];

    if (nick.size() > 9) {
        std::string current = clients[fd].getNickname().empty() ? "*" : clients[fd].getNickname();
        sendToClient(fd, " 432 " + current + " " + nick + " :Erroneous nickname\r\n");
        return ;
    }
    std::string specialChar = "[]\\^_{}|";
    if (!std::isalpha(nick[0]) && specialChar.find(nick[0]) == std::string::npos) {
        std::string current = clients[fd].getNickname().empty() ? "*" : clients[fd].getNickname();
        sendToClient(fd, " 432 " + current + " " + nick + " :Erroneous nickname\r\n");
        return;
    }
    for (size_t i = 1; i < nick.size(); ++i) {
        if (!std::isalnum(nick[i]) && nick[i] != '-' && specialChar.find(nick[i]) == std::string::npos) {
            std::string current = clients[fd].getNickname().empty() ? "*" : clients[fd].getNickname();
            sendToClient(fd, " 432 " + current + " " + nick + " :Erroneous nickname\r\n");
            return;
        }
    }

    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->first != fd && it->second.getNickname() == nick) {
            sendToClient(fd, " 433 * " + nick + " :Nickname is already in use\r\n");
            return;
        }
    }

    if (!clients[fd].registered()) {
        clients[fd].setNickname(nick);
        if (clients[fd].registered())
            sendToClient(fd, " 001 " + clients[fd].getNickname() + " :Welcome to the IRC server " +
                         clients[fd].getNickname() + "!" + clients[fd].getUsername() + "@localhost\r\n");
    } else {
        std::string oldnick = clients[fd].getNickname();
        clients[fd].setNickname(nick);
        std::string msg = ":" + oldnick + "!" + clients[fd].getUsername() + "@localhost NICK " + nick + "\r\n";
        for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
            sendRaw(it->first, msg);
    }
}