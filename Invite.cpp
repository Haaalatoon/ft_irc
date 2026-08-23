#include "Server.hpp"
#include "ErrorReply.hpp"

void Server::handleInvite(int fd, std::vector<std::string> args) {
    Client* client = getClientPtr(fd);
    if (!client) return;
    if (!client->registered()) {
        sendToClient(fd, " 451 * :You have not registered\r\n");
        return;
    }
    if (args.size() < 2) {
        sendToClient(fd, ERR_NEEDMOREPARAMS(client->getNickname(), "INVITE"));
        return;
    }

    std::string targetNick = args[0];
    std::string chanName = args[1];

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
    if (channel->isMember(target->getFd())) {
        sendToClient(fd, " 443 " + client->getNickname() + " " + targetNick + " " + chanName + " :is already on channel\r\n");
        return;
    }

    channel->addInvite(target->getFd());
    sendToClient(fd, " 341 " + client->getNickname() + " " + targetNick + " " + chanName + "\r\n");
    std::string inviteMsg = ":" + client->getPrefix() + " INVITE " + targetNick + " " + chanName + "\r\n";
    sendRaw(target->getFd(), inviteMsg);
}