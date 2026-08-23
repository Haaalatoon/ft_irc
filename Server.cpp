#include "Server.hpp"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

bool Server::_running = false;

Server::Server(int p, const std::string& pass)
    : port(p), _fd(-1), serverName("ircserv"), password(pass) {
    memset(&addr, 0, sizeof(addr));
}

Server::~Server() {}

void Server::start() {
    signal(SIGINT, Server::signalHandler);
    initSocket();
    _running = true;
    while (_running) {
        int ret = poll(&pollFD[0], pollFD.size(), -1);
        if (ret == -1) {
            if (errno == EINTR) continue;
            throw std::runtime_error("poll failed");
        }
        for (size_t i = 0; i < pollFD.size(); ++i) {
            if (pollFD[i].revents & (POLLHUP | POLLERR)) {
                removeClient(pollFD[i].fd);
                --i;
                continue;
            }
            if (pollFD[i].revents & POLLIN) {
                if (pollFD[i].fd == _fd) {
                    while (true) {
                        int newfd = acceptClientInternal();
                        if (newfd == -1) break;
                    }
                } else {
                    if (handleClientData(i)) --i;
                }
            }
            if (pollFD[i].revents & POLLOUT) {
                int fd = pollFD[i].fd;
                std::map<int, Client>::iterator it = clients.find(fd);
                if (it != clients.end()) {
                    std::string& out = it->second.getOutBuffer();
                    if (out.size() > 512 * 1024) {
                        removeClient(fd); 
                        --i; 
                        continue;
                    }
                    if (!out.empty()) {
                        ssize_t sent = send(fd, out.c_str(), out.size(), 0);
                        if (sent >= 0)
                            out.erase(0, sent);
                        else if (errno != EWOULDBLOCK)
                            out.clear();
                    }
                    if (out.empty()) {
                        for (size_t j = 0; j < pollFD.size(); ++j)
                            if (pollFD[j].fd == fd) { pollFD[j].events &= ~POLLOUT; break; }
                    }
                }
            }
        }
    }
    for (size_t i = 0; i < pollFD.size(); ++i)
        if (pollFD[i].fd != _fd) removeClient(pollFD[i].fd);
    for (std::map<std::string, Channel*>::iterator it = channels.begin();
         it != channels.end(); ++it)
        delete it->second;
    channels.clear();
    close(_fd);
}

void Server::initSocket() {
    int opt = 1;
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd == -1) throw std::runtime_error("socket() failed");
    setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    fcntl(_fd, F_SETFL, O_NONBLOCK);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    bindAndListen();
    struct pollfd pfd;
    pfd.fd = _fd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    pollFD.push_back(pfd);
}

void Server::bindAndListen() {
    if (bind(_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
        throw std::runtime_error("bind() failed");
    if (listen(_fd, SOMAXCONN) == -1)
        throw std::runtime_error("listen() failed");
}

int Server::acceptClientInternal() {
    socklen_t len = sizeof(addr);
    int newfd = accept(_fd, (struct sockaddr*)&addr, &len);
    if (newfd == -1) {
        if (errno != EWOULDBLOCK && errno != EAGAIN)
            std::cerr << "accept error: " << strerror(errno) << std::endl;
        return -1;
    }
    fcntl(newfd, F_SETFL, O_NONBLOCK);
    struct pollfd pfd;
    pfd.fd = newfd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    pollFD.push_back(pfd);
    clients.insert(std::make_pair(newfd, Client(newfd)));
    std::cout << "Client connected: fd " << newfd << std::endl;
    return newfd;
}

void Server::removeClient(int FD) {
    std::vector<std::string> toDelete;
    for (std::map<std::string, Channel*>::iterator it = channels.begin();
         it != channels.end(); ++it) {
        it->second->removeMember(FD);
        it->second->removeInvite(FD);
        if (it->second->getMemberCount() == 0)
            toDelete.push_back(it->first);
    }
    for (size_t i = 0; i < toDelete.size(); ++i)
        removeChannel(toDelete[i]);

    close(FD);
    for (size_t i = 0; i < pollFD.size(); ++i) {
        if (pollFD[i].fd == FD) {
            pollFD.erase(pollFD.begin() + i);
            break;
        }
    }
    clients.erase(FD);
    std::cout << "Client disconnected: fd " << FD << std::endl;
}

void Server::parseMessages(int fd) {
    std::map<int, Client>::iterator it = clients.find(fd);
    if (it == clients.end()) return;
    std::string& buf = it->second.getBuffer();
    size_t pos;
    while ((pos = buf.find('\n')) != std::string::npos) {
        std::string msg = buf.substr(0, pos);
        buf.erase(0, pos + 1);
        if (!msg.empty() && msg[msg.size() - 1] == '\r')
            msg.erase(msg.size() - 1);
        if (!msg.empty())
            parseCommand(fd, msg);
    }
}

bool Server::handleClientData(size_t& index) {
    int fd = pollFD[index].fd;
    char temp[1024];
    int rc = recv(fd, temp, sizeof(temp), 0);
    if (rc <= 0) {
        if (rc == 0 || errno != EWOULDBLOCK) {
            removeClient(fd);
            return true;
        }
        return false;
    }
    clients[fd].appendToBuffer(temp, rc);
    parseMessages(fd);
    return false;
}

void Server::sendToClient(int fd, const std::string& message) {
    std::map<int, Client>::iterator it = clients.find(fd);
    if (it == clients.end()) return;
    it->second.getOutBuffer() += ":" + serverName + message;
    for (size_t i = 0; i < pollFD.size(); ++i)
        if (pollFD[i].fd == fd) { pollFD[i].events |= POLLOUT; break; }
}

void Server::sendRaw(int fd, const std::string& message) {
    std::map<int, Client>::iterator it = clients.find(fd);
    if (it == clients.end()) return;
    it->second.getOutBuffer() += message;
    for (size_t i = 0; i < pollFD.size(); ++i)
        if (pollFD[i].fd == fd) { pollFD[i].events |= POLLOUT; break; }
}

void Server::broadcastRaw(Channel* channel, const std::string& msg, int excludeFd) {
    const std::vector<int>& users = channel->getUsers();
    const std::vector<int>& ops = channel->getOperators();
    for (size_t i = 0; i < ops.size(); ++i)
        if (ops[i] != excludeFd) sendRaw(ops[i], msg);
    for (size_t i = 0; i < users.size(); ++i)
        if (users[i] != excludeFd) sendRaw(users[i], msg);
}

Client* Server::getClientPtr(int fd) {
    std::map<int, Client>::iterator it = clients.find(fd);
    return (it != clients.end()) ? &it->second : NULL;
}

Client* Server::getClientByNick(const std::string& nick) {
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
        if (it->second.getNickname() == nick)
            return &it->second;
    return NULL;
}

Channel* Server::getChannel(const std::string& name) {
    std::map<std::string, Channel*>::iterator it = channels.find(name);
    return (it != channels.end()) ? it->second : NULL;
}

void Server::addChannel(Channel* ch) { channels[ch->getName()] = ch; }

void Server::removeChannel(const std::string& name) {
    std::map<std::string, Channel*>::iterator it = channels.find(name);
    if (it != channels.end()) { delete it->second; channels.erase(it); }
}

int Server::getUserChannelCount(int fd) {
    int count = 0;
    for (std::map<std::string, Channel*>::iterator it = channels.begin();
         it != channels.end(); ++it)
        if (it->second->isMember(fd))
            count++;
    return count;
}

std::string Server::buildNames(Channel* channel) {
    std::string names;
    const std::vector<int>& ops = channel->getOperators();
    for (size_t i = 0; i < ops.size(); ++i) {
        Client* c = getClientPtr(ops[i]);
        if (c) {
            if (!names.empty()) names += " ";
            names += "@" + c->getNickname();
        }
    }
    const std::vector<int>& users = channel->getUsers();
    for (size_t i = 0; i < users.size(); ++i) {
        Client* c = getClientPtr(users[i]);
        if (c) {
            if (!names.empty()) names += " ";
            names += c->getNickname();
        }
    }
    return names;
}

void Server::signalHandler(int) { _running = false; }

void Server::parseCommand(int fd, const std::string& msg) {
    std::string cmd;
    std::vector<std::string> args;

    size_t pos = msg.find(' ');
    if (pos == std::string::npos) {
        cmd = msg;
    } else {
        cmd = msg.substr(0, pos);
        std::string rest = msg.substr(pos + 1);
        size_t start = 0;
        while (start < rest.size() && rest[start] == ' ')
            ++start;
        rest = rest.substr(start);

        while (!rest.empty()) {
            if (rest[0] == ':') {
                args.push_back(rest.substr(1));
                break;
            }
            size_t spacePos = rest.find(' ');
            if (spacePos == std::string::npos) {
                args.push_back(rest);
                break;
            }
            args.push_back(rest.substr(0, spacePos));
            rest = rest.substr(spacePos + 1);
            size_t i = 0;
            while (i < rest.size() && rest[i] == ' ') ++i;
            rest = rest.substr(i);
        }
    }
    dispatch(fd, cmd, args);
}

void Server::dispatch(int fd, const std::string& cmd, std::vector<std::string>& args) {
    std::string upperCmd = cmd;
    for (size_t i = 0; i < upperCmd.size(); ++i)
        upperCmd[i] = std::toupper(upperCmd[i]);

    void (Server::*handlers[])(int, std::vector<std::string>) = {
        &Server::handlePass,   &Server::handleNick,   &Server::handleUser,
        &Server::handleJoin,   &Server::handlePart,   &Server::handlePrivmsg,
        &Server::handleKick,   &Server::handleInvite, &Server::handleTopic,
        &Server::handleMode,   &Server::handleQuit,   &Server::handlePing,
        &Server::handleCap
    };
    std::string cmds[] = {
        "PASS", "NICK", "USER", "JOIN", "PART", "PRIVMSG",
        "KICK", "INVITE", "TOPIC", "MODE", "QUIT", "PING",
        "CAP"
    };
    const int count = sizeof(cmds) / sizeof(cmds[0]);

    for (int i = 0; i < count; ++i) {
        if (upperCmd == cmds[i]) {
            (this->*handlers[i])(fd, args);
            return;
        }
    }

    std::string nick = "*";
    std::map<int, Client>::iterator it = clients.find(fd);
    if (it != clients.end())
        nick = it->second.getNickname();
    sendToClient(fd, " 421 " + nick + " " + upperCmd + " :Unknown command\r\n");
}