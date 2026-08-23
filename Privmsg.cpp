#include "Server.hpp"
#include "ErrorReply.hpp"
#include <sstream>

static std::vector<std::string> splitComma(const std::string& str) {
    std::vector<std::string> result;
    std::string token;
    std::stringstream ss(str);
    while (std::getline(ss, token, ','))
        if (!token.empty()) result.push_back(token);
    return result;
}

void Server::handlePrivmsg(int fd, std::vector<std::string> args) {
    Client* client = getClientPtr(fd);
    if (!client) return;
    if (!client->registered()) {
        sendToClient(fd, " 451 * :You have not registered\r\n");
        return;
    }
    if (args.empty()) {
        sendToClient(fd, " 411 " + client->getNickname() + " :No recipient given (PRIVMSG)\r\n");
        return;
    }
    if (args.size() < 2) {
        sendToClient(fd, " 412 " + client->getNickname() + " :No text to send\r\n");
        return;
    }

    std::vector<std::string> targets = splitComma(args[0]);
    if (targets.size() > 10) {
        sendToClient(fd, " 407 " + client->getNickname() + " :Too many recipients\r\n");
        return;
    }
    std::string message = args[1];

    for (size_t i = 0; i < targets.size(); ++i) {
        std::string target = targets[i];
        if (target.empty()) continue;

        if (target[0] == '#') {
            Channel* channel = getChannel(target);
            if (!channel) {
                sendToClient(fd, ERR_NOSUCHCHANNEL(client->getNickname(), target));
                continue;
            }
            if (!channel->isMember(fd)) {
                sendToClient(fd, " 404 " + client->getNickname() + " " + target + " :Cannot send to channel\r\n");
                continue;
            }
            std::string msg = ":" + client->getPrefix() + " PRIVMSG " + target + " :" + message + "\r\n";
            broadcastRaw(channel, msg, fd);
        } else {
            Client* targetClient = getClientByNick(target);
            if (!targetClient) {
                sendToClient(fd, ERR_NOSUCHNICK(client->getNickname(), target));
                continue;
            }
            std::string msg = ":" + client->getPrefix() + " PRIVMSG " + target + " :" + message + "\r\n";
            sendRaw(targetClient->getFd(), msg);
        }
    }
}