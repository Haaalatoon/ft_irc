#include "Server.hpp"
#include "ErrorReply.hpp"
#include <cstdlib>
#include <sstream>

namespace {

struct ModeAction {
    char        mode;
    char        sign;
    std::string arg;
    ModeAction(char m, char s, const std::string& a = "")
        : mode(m), sign(s), arg(a) {}
};

} // anonymous namespace

void Server::handleMode(int fd, std::vector<std::string> args) {
    Client* client = getClientPtr(fd);
    if (!client) return;
    if (!client->registered()) {
        sendToClient(fd, " 451 * :You have not registered\r\n");
        return;
    }
    if (args.empty()) {
        sendToClient(fd, ERR_NEEDMOREPARAMS(client->getNickname(), "MODE"));
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

    // VIEW MODE
    if (args.size() < 2) {
        std::string modeStr = "+";
        std::string modeParams;
        if (channel->isInviteOnly()) modeStr += "i";
        if (channel->isTopicRestricted()) modeStr += "t";
        if (channel->hasPassword()) {
            modeStr += "k";
            modeParams += " " + channel->getPassword();
        }
        if (channel->hasUserLimit()) {
            modeStr += "l";
            std::ostringstream oss;
            oss << channel->getUserLimit();
            modeParams += " " + oss.str();
        }
        sendToClient(fd, " 324 " + client->getNickname() + " " + chanName + " " + modeStr + modeParams + "\r\n");
        return;
    }
    // SET MODE
    if (!channel->isOperator(fd)) {
        sendToClient(fd, ERR_CHANOPRIVSNEEDED(client->getNickname(), chanName));
        return;
    }

    std::string modestr = args[1];
    if (modestr.empty() || (modestr[0] != '+' && modestr[0] != '-')) {
        sendToClient(fd, ERR_UNKNOWNMODE(client->getNickname(), modestr));
        return;
    }

    // First pass: validate
    char op = '+';
    size_t argIdx = 2;
    std::vector<ModeAction> actions;

    for (size_t i = 0; i < modestr.size(); ++i) {
        char c = modestr[i];
        if (c == '+' || c == '-') { op = c; continue; }

        std::string neededArg;
        if (c == 'k' || c == 'l' || c == 'o') {
            if (op == '+') {
                if (argIdx >= args.size()) {
                    sendToClient(fd, ERR_NEEDMOREPARAMS(client->getNickname(), "MODE"));
                    return;
                }
                neededArg = args[argIdx++];
            } else if (op == '-' && c == 'o') {
                if (argIdx >= args.size()) {
                    sendToClient(fd, ERR_NEEDMOREPARAMS(client->getNickname(), "MODE"));
                    return;
                }
                neededArg = args[argIdx++];
            }
        }

        if (c != 'i' && c != 't' && c != 'k' && c != 'l' && c != 'o') {
            sendToClient(fd, ERR_UNKNOWNMODE(client->getNickname(), std::string(1, c)));
            return;
        }

        if (c == 'k' && op == '+') {
            if (neededArg.empty()) {
                sendToClient(fd, ERR_NEEDMOREPARAMS(client->getNickname(), "MODE +k"));
                return;
            }
            if (channel->hasPassword()) {
                sendToClient(fd, ERR_KEYSET(client->getNickname(), chanName));
                return;
            }
        }
        if (c == 'l' && op == '+') {
            int limit = std::atoi(neededArg.c_str());
            if (limit <= 0) {
                sendToClient(fd, ERR_NEEDMOREPARAMS(client->getNickname(), "MODE +l"));
                return;
            }
        }
        if (c == 'o') {
            Client* target = getClientByNick(neededArg);
            if (!target || !channel->isMember(target->getFd())) {
                sendToClient(fd, ERR_USERNOTINCHANNEL(client->getNickname(), chanName, neededArg));
                return;
            }
        }

        actions.push_back(ModeAction(c, op, neededArg));
    }

    // Second pass: apply and broadcast
    std::string broadcastFlags;
    std::string broadcastArgs;
    char lastSign = 0;

    for (size_t i = 0; i < actions.size(); ++i) {
        ModeAction& a = actions[i];
        char c = a.mode;
        char sign = a.sign;

        if (broadcastFlags.empty() || lastSign != sign) {
            broadcastFlags += sign;
            lastSign = sign;
        }
        broadcastFlags += c;

        switch (c) {
            case 'i': channel->setInviteOnly(sign == '+'); break;
            case 't': channel->setTopicRestricted(sign == '+'); break;
            case 'k':
                if (sign == '+') {
                    channel->setPassword(a.arg);
                    broadcastArgs += " " + a.arg;
                } else {
                    if (channel->hasPassword()) channel->removePassword();
                }
                break;
            case 'l':
                if (sign == '+') {
                    int limit = std::atoi(a.arg.c_str());
                    channel->setUserLimit(limit);
                    broadcastArgs += " " + a.arg;
                } else {
                    channel->removeUserLimit();
                }
                break;
            case 'o': {
                Client* target = getClientByNick(a.arg);
                if (sign == '+')
                    channel->addOperator(target->getFd());
                else
                    channel->removeOperator(target->getFd());
                broadcastArgs += " " + a.arg;
                break;
            }
        }
    }
    if (!broadcastFlags.empty()) {
        std::string modeChange = ":" + client->getPrefix() + " MODE " + chanName + " " + broadcastFlags + broadcastArgs + "\r\n";
        broadcastRaw(channel, modeChange, -1);
    }
}