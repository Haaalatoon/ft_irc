#include "Server.hpp"

void Server::handleQuit(int fd, std::vector<std::string> args) {
    Client* client = getClientPtr(fd);
    if (!client) return;

    std::string reason = args.empty() ? "Client quit" : args[0];
    std::string quitMsg = ":" + client->getPrefix() + " QUIT :" + reason + "\r\n";

    for (std::map<std::string, Channel*>::iterator it = channels.begin();
         it != channels.end(); ++it) {
        if (it->second->isMember(fd))
            broadcastRaw(it->second, quitMsg, fd);
    }
    removeClient(fd);
}