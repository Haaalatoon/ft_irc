#include "Server.hpp"
#include "ErrorReply.hpp"

void Server::handleKick(int fd, std::vector<std::string> args) {
    Client* client = getClientPtr(fd);
    if (!client) return;
    if (!client->registered()) {
        sendToClient(fd, " 451 * :You have not registered\r\n");
        return;
    }
    if (args.size() < 2) {
        sendToClient(fd, ERR_NEEDMOREPARAMS(client->getNickname(), "KICK"));
        return;
    }

    std::string chanName = args[0];
    std::string targetNick = args[1];
    std::string reason;
    if (args.size() > 2) reason = args[2];

    Channel* channel = getChannel(chanName);
    if (!channel) {
        sendToClient(fd, ERR_NOSUCHCHANNEL(client->getNickname(), chanName));
        return;
    }
    if (!channel->isMember(fd)) {
        sendToClient(fd, ERR_NOTONCHANNEL(client->getNickname(), chanName));
        return;
    }
    if (!channel->isOperator(fd)) {
        sendToClient(fd, ERR_CHANOPRIVSNEEDED(client->getNickname(), chanName));
        return;
    }

    Client* target = getClientByNick(targetNick);
    if (!target) {
        sendToClient(fd, ERR_NOSUCHNICK(client->getNickname(), targetNick));
        return;
    }
    if (!channel->isMember(target->getFd())) {
        sendToClient(fd, ERR_USERNOTINCHANNEL(client->getNickname(), chanName, targetNick));
        return;
    }

    std::string kickMsg = ":" + client->getPrefix() + " KICK " + chanName + " " + targetNick;
    if (!reason.empty()) kickMsg += " :" + reason;
    kickMsg += "\r\n";

    broadcastRaw(channel, kickMsg, -1);
    channel->removeMember(target->getFd());
    if (channel->getMemberCount() == 0)
        removeChannel(chanName);
}