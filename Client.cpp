#include "Client.hpp"

Client::Client() : fd(-1), isAuthenticated(false) {}
Client::Client(int FD) : fd(FD), isAuthenticated(false) {}
Client::Client(const Client& other)
    : fd(other.fd),
      nickname(other.nickname),
      username(other.username),
      realname(other.realname),
      buffer(other.buffer),
      outBuffer(other.outBuffer),
      isAuthenticated(other.isAuthenticated) {}
Client& Client::operator=(const Client& other) {
    if (this != &other) {
        fd = other.fd;
        nickname = other.nickname;
        username = other.username;
        realname = other.realname;
        buffer = other.buffer;
        outBuffer = other.outBuffer;
        isAuthenticated = other.isAuthenticated;
    }
    return *this;
}
Client::~Client() {}

std::string Client::getNickname() const { return nickname; }
void Client::setUsername(const std::string& name) { username = name; }
std::string Client::getUsername() const { return username; }
void Client::setRealname(const std::string& name) { realname = name; }
bool Client::getisAuthenticated() const { return isAuthenticated; }
void Client::setisAuthenticated(bool value) { isAuthenticated = value; }
std::string& Client::getBuffer() { return buffer; }
void Client::setBuffer(const std::string& buf) { buffer = buf; }
void Client::setNickname(const std::string& nick) { nickname = nick; }

void Client::appendToBuffer(const char* buf, int size) {
    buffer.append(buf, size);
}

bool Client::registered() const {
    return (isAuthenticated && !nickname.empty() && !username.empty());
}

int Client::getFd() const { return fd; }

std::string Client::getPrefix() const {
    return nickname + "!~" + username + "@localhost";
}

std::string& Client::getOutBuffer() { return outBuffer; }

void Client::clearOutBuffer() { outBuffer.clear(); }