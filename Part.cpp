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

void Server::handlePart(int fd, std::vector<std::string> args) {
    Client* client = getClientPtr(fd);
    if (!client) return;
    if (!client->registered()) {
        sendToClient(fd, " 451 * :You have not registered\r\n");
        return;
    }
    if (args.empty()) {
        sendToClient(fd, ERR_NEEDMOREPARAMS(client->getNickname(), "PART"));
        return;
    }

    std::vector<std::string> chanNames = splitComma(args[0]);
    std::string reason;
    if (args.size() > 1) reason = args[1];

    for (size_t i = 0; i < chanNames.size(); ++i) {
        std::string chanName = chanNames[i];
        if (chanName.empty() || chanName[0] != '#') {
            sendToClient(fd, ERR_NOSUCHCHANNEL(client->getNickname(), chanName));
            continue;
        }

        Channel* channel = getChannel(chanName);
        if (!channel) {
            sendToClient(fd, ERR_NOSUCHCHANNEL(client->getNickname(), chanName));
            continue;
        }
        if (!channel->isMember(fd)) {
            sendToClient(fd, ERR_NOTONCHANNEL(client->getNickname(), chanName));
            continue;
        }

        std::string partMsg = ":" + client->getPrefix() + " PART " + chanName;
        if (!reason.empty()) partMsg += " :" + reason;
        partMsg += "\r\n";

        broadcastRaw(channel, partMsg, -1);
        channel->removeMember(fd);

        if (channel->getMemberCount() == 0)
            removeChannel(chanName);
    }
}