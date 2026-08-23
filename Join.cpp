#include "Server.hpp"
#include "ErrorReply.hpp"
#include <sstream>
#include <algorithm>

static bool isValidChannelName(const std::string& name) {
    if (name.empty() || name[0] != '#' || name.size() > 50)
        return false;

    for (size_t i = 1; i < name.size(); ++i) {
        unsigned char c = name[i];
        if (c <= 32 || c == ',' || c == 127)
            return false;
    }

    return true;
}

static std::vector<std::string> splitComma(const std::string& str) {
    std::vector<std::string> result;
    std::string token;
    std::stringstream ss(str);

    while (std::getline(ss, token, ','))
        if (!token.empty())
            result.push_back(token);

    return result;
}

void Server::handleJoin(int fd, std::vector<std::string> args) {
    Client* client = getClientPtr(fd);

    if (!client)
        return;

    if (!client->registered()) {
        sendToClient(fd, " 451 * :You have not registered\r\n");
        return;
    }
    if (args.empty()) {
        sendToClient(fd, ERR_NEEDMOREPARAMS(client->getNickname(), "JOIN"));
        return;
    }

    std::vector<std::string> chanNames = splitComma(args[0]);
    std::vector<std::string> passwords;

    if (args.size() > 1)
        passwords = splitComma(args[1]);

    for (size_t i = 0; i < chanNames.size(); ++i) {
        std::string chanName = chanNames[i];
        std::string password = (i < passwords.size()) ? passwords[i] : "";

        if (!isValidChannelName(chanName)) {
            sendToClient(fd, ERR_NOSUCHCHANNEL(client->getNickname(), chanName));
            continue;
        }
        if (getUserChannelCount(fd) >= 10) {
            sendToClient(fd, " 405 " + client->getNickname() + " " + chanName + " :You have joined too many channels\r\n");
            continue;
        }

        Channel* channel = getChannel(chanName);
        if (!channel) {
            channel = new Channel(chanName);

            addChannel(channel);
            channel->addOperator(fd);
        } else {
            if (channel->isMember(fd))
                continue;
            if (channel->isInviteOnly() && !channel->isInvited(fd)) {
                sendToClient(fd, " 473 " + client->getNickname() + " " + chanName + " :Cannot join channel (+i)\r\n");
                continue;
            }
            if (channel->hasPassword() && channel->getPassword() != password) {
                sendToClient(fd, " 475 " + client->getNickname() + " " + chanName + " :Cannot join channel (+k)\r\n");
                continue;
            }
            if (channel->hasUserLimit() && channel->getMemberCount() >= channel->getUserLimit()) {
                sendToClient(fd, " 471 " + client->getNickname() + " " + chanName + " :Cannot join channel (+l)\r\n");
                continue;
            }
            channel->addMember(fd);
            channel->removeInvite(fd);
        }

        std::string joinMsg = ":" + client->getPrefix() + " JOIN " + chanName + "\r\n";
        broadcastRaw(channel, joinMsg, -1);

        if (!channel->getTopic().empty())
            sendToClient(fd, " 332 " + client->getNickname() + " " + chanName + " :" + channel->getTopic() + "\r\n");
        else
            sendToClient(fd, " 331 " + client->getNickname() + " " + chanName + " :No topic is set\r\n");

        sendToClient(fd, " 353 " + client->getNickname() + " = " + chanName + " :" + buildNames(channel) + "\r\n");
        sendToClient(fd, " 366 " + client->getNickname() + " " + chanName + " :End of /NAMES list\r\n");
    }
}