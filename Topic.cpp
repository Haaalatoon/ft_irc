#include "Server.hpp"
#include "ErrorReply.hpp"

void Server::handleTopic(int fd, std::vector<std::string> args) {
    Client* client = getClientPtr(fd);
    if (!client) return;
    if (!client->registered()) {
        sendToClient(fd, " 451 * :You have not registered\r\n");
        return;
    }
    if (args.empty()) {
        sendToClient(fd, ERR_NEEDMOREPARAMS(client->getNickname(), "TOPIC"));
        return;
    }

    std::string chanName = args[0];
    Channel* channel = getChannel(chanName);
    if (!channel) {
        sendToClient(fd, ERR_NOSUCHCHANNEL(client->getNickname(), chanName));
        return;
    }
    if (!channel->isMember(fd)) {
        sendToClient(fd, ERR_NOTONCHANNEL(client->getNickname(), chanName));
        return;
    }

    if (args.size() < 2) {
        if (channel->getTopic().empty())
            sendToClient(fd, " 331 " + client->getNickname() + " " + chanName + " :No topic is set\r\n");
        else
            sendToClient(fd, " 332 " + client->getNickname() + " " + chanName + " :" + channel->getTopic() + "\r\n");
        return;
    }

    std::string newTopic = args[1];
    if (channel->isTopicRestricted() && !channel->isOperator(fd)) {
        sendToClient(fd, ERR_CHANOPRIVSNEEDED(client->getNickname(), chanName));
        return;
    }

    channel->setTopic(newTopic);
    std::string topicMsg = ":" + client->getPrefix() + " TOPIC " + chanName + " :" + newTopic + "\r\n";
    broadcastRaw(channel, topicMsg, -1);
}