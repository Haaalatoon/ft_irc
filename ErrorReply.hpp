#ifndef ERRORREPLY_HPP
#define ERRORREPLY_HPP

#include <string>

#define ERR_NEEDMOREPARAMS(nick, cmd) \
    (" 461 " + nick + " " + cmd + " :Not enough parameters\r\n")
#define ERR_NOSUCHCHANNEL(nick, chname) \
    (" 403 " + nick + " " + chname + " :No such channel\r\n")
#define ERR_NOTONCHANNEL(nick, chname) \
    (" 442 " + nick + " " + chname + " :You're not on that channel\r\n")
#define ERR_CHANOPRIVSNEEDED(nick, chname) \
    (" 482 " + nick + " " + chname + " :You're not channel operator\r\n")
#define ERR_USERNOTINCHANNEL(nick, chname, target) \
    (" 441 " + nick + " " + target + " " + chname + " :They aren't on that channel\r\n")
#define ERR_NOSUCHNICK(nick, target) \
    (" 401 " + nick + " " + target + " :No such nick/channel\r\n")
#define ERR_UNKNOWNMODE(nick, modechar) \
    (" 472 " + nick + " " + modechar + " :is unknown mode char to me\r\n")
#define ERR_KEYSET(nick, chname) \
    (" 467 " + nick + " " + chname + " :Channel key already set\r\n")

#endif