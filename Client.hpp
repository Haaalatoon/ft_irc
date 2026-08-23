#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client {
private:
    int         fd;
    std::string nickname;
    std::string username;
    std::string realname;
    std::string buffer;
    std::string outBuffer;
    bool        isAuthenticated;

public:
    Client();
    Client(int FD);
    ~Client();

    std::string getNickname() const;
    void        setUsername(const std::string& name);
    std::string getUsername() const;
    void        setRealname(const std::string& name);
    bool        getisAuthenticated() const;
    void        setisAuthenticated(bool value);
    std::string& getBuffer();
    void        setBuffer(const std::string& buf);
    void        setNickname(const std::string& nick);
    void        appendToBuffer(const char* buf, int size);
    bool        registered() const;

    int         getFd() const;
    std::string getPrefix() const;

    std::string& getOutBuffer();
    void         clearOutBuffer();
};

#endif