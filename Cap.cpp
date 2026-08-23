#include "Server.hpp"

void Server::handleCap(int fd, std::vector<std::string> args) {
    if (args.empty()) return;
    std::string subcmd = args[0];
    if (subcmd == "LS") {
        std::string reply = ":" + serverName + " CAP * LS :\r\n";
        sendRaw(fd, reply);
    } else if (subcmd == "END") {
        // nothing
    } else if (subcmd == "REQ") {
        std::string reply = ":" + serverName + " CAP * NAK :\r\n";
        sendRaw(fd, reply);
    }
}