#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include "Channel.hpp"
#include <vector>
#include <algorithm>  
#include <map>
#include <poll.h>
#include <netinet/in.h>
#include <string>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

class Server {
private:
    int                         port;
    int                         _fd;
    std::string                 serverName;
    std::string                 password;
    sockaddr_in                 addr;
    std::vector<struct pollfd>  pollFD;
    std::map<int, Client>       clients;
    std::map<std::string, Channel*> channels;
    static bool                 _running;

    void parseCommand(int fd, const std::string& msg);
    void dispatch(int fd, const std::string& cmd, std::vector<std::string>& args);

    void initSocket();
    void bindAndListen();
    int  acceptClientInternal();
    bool handleClientData(size_t& index);
    void removeClient(int FD);
    void parseMessages(int fd);

public:
    Server(int p, const std::string& pass);
    ~Server();
    void start();

    Client*  getClientPtr(int fd);
    Client*  getClientByNick(const std::string& nick);
    Channel* getChannel(const std::string& name);
    void     addChannel(Channel* ch);
    void     removeChannel(const std::string& name);
    int      getUserChannelCount(int fd);

    void sendToClient(int fd, const std::string& message);
    void sendRaw(int fd, const std::string& message);
    void broadcastRaw(Channel* channel, const std::string& msg, int excludeFd);

    void handlePass(int fd, std::vector<std::string> args);
    void handleNick(int fd, std::vector<std::string> args);
    void handleUser(int fd, std::vector<std::string> args);
    void handleQuit(int fd, std::vector<std::string> args);
    void handleJoin(int fd, std::vector<std::string> args);
    void handlePart(int fd, std::vector<std::string> args);
    void handlePrivmsg(int fd, std::vector<std::string> args);
    void handleKick(int fd, std::vector<std::string> args);
    void handleInvite(int fd, std::vector<std::string> args);
    void handleTopic(int fd, std::vector<std::string> args);
    void handleMode(int fd, std::vector<std::string> args);
    void handlePing(int fd, std::vector<std::string> args);
    void handleCap(int fd, std::vector<std::string> args);

    std::string buildNames(Channel* channel);

    static void signalHandler(int signum);
};

#endif