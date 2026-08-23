#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <ctime>

class Channel {
private:
    std::string              _name;
    std::string              _topic;
    std::string              _password;
    std::string              _creationTime;
    int                      _userLimit;
    bool                     _inviteOnly;
    bool                     _topicRestricted;
    bool                     _hasPassword;
    bool                     _hasUserLimit;

    std::vector<int>         _users;
    std::vector<int>         _operators;
    std::vector<int>         _inviteList;

    Channel(const Channel& other);
    Channel& operator=(const Channel& other);

public:
    Channel(const std::string& name);
    ~Channel();

    void addMember(int fd);
    void removeMember(int fd);
    bool isMember(int fd)  const;
    int  getMemberCount()  const;
    const std::vector<int>& getUsers()     const;
    const std::vector<int>& getOperators() const;

    void addOperator(int fd);
    void removeOperator(int fd);
    bool isOperator(int fd) const;
    void addInvite(int fd);
    void removeInvite(int fd);
    bool isInvited(int fd) const;

    const std::string& getName()           const;
    const std::string& getTopic()          const;
    const std::string& getPassword()       const;
    const std::string& getCreationTime()   const;
    int                getUserLimit()      const;
    bool               isInviteOnly()      const;
    bool               isTopicRestricted() const;
    bool               hasPassword()       const;
    bool               hasUserLimit()      const;

    void setTopic(const std::string& t);
    void setPassword(const std::string& p);
    void removePassword();
    void setUserLimit(int l);
    void removeUserLimit();
    void setInviteOnly(bool v);
    void setTopicRestricted(bool v);
};

#endif