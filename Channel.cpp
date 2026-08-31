#include "Channel.hpp"

Channel::Channel(const std::string& name)
    : _name(name), _userLimit(0), _inviteOnly(false),
      _topicRestricted(false), _hasPassword(false), _hasUserLimit(false) {
    std::time_t t = std::time(NULL);
    std::ostringstream oss;
    oss << t;
    _creationTime = oss.str();
}

Channel::~Channel() {}

void Channel::addMember(int fd) {
    if (!isMember(fd))
        _users.push_back(fd);
}

void Channel::removeMember(int fd) {
    std::vector<int>::iterator it;

    it = std::find(_users.begin(), _users.end(), fd);
    if (it != _users.end()) {
         _users.erase(it); 
         return ; 
    }

    it = std::find(_operators.begin(), _operators.end(), fd);
    
    if (it != _operators.end()) { 
        _operators.erase(it); 
        return ;
    }
    
    // ✅ Also check invite list (cleanup)
    it = std::find(_inviteList.begin(), _inviteList.end(), fd);
    if (it != _inviteList.end()) {
        _inviteList.erase(it);
    }
}

bool Channel::isMember(int fd) const {
    return (std::find(_users.begin(), _users.end(), fd) != _users.end()
        || std::find(_operators.begin(), _operators.end(), fd) != _operators.end());
}

int Channel::getMemberCount() const {
    return (static_cast<int>(_users.size() + _operators.size()));
}

const std::vector<int>& Channel::getUsers()     const { return _users; }
const std::vector<int>& Channel::getOperators() const { return _operators; }

void Channel::addOperator(int fd) {
    std::vector<int>::iterator it = std::find(_users.begin(), _users.end(), fd);

    if (it != _users.end())
        _users.erase(it);
    if (!isOperator(fd))
        _operators.push_back(fd);
}

void Channel::removeOperator(int fd) {
    std::vector<int>::iterator it = std::find(_operators.begin(), _operators.end(), fd);
    if (it != _operators.end()) {
        _operators.erase(it);
        _users.push_back(fd);
    }
}

bool Channel::isOperator(int fd) const {
    return (std::find(_operators.begin(), _operators.end(), fd) != _operators.end());
}

void Channel::addInvite(int fd) {
    if (!isInvited(fd))
        _inviteList.push_back(fd);
}

void Channel::removeInvite(int fd) {
    std::vector<int>::iterator it = std::find(_inviteList.begin(), _inviteList.end(), fd);

    if (it != _inviteList.end())
        _inviteList.erase(it);
}
bool Channel::isInvited(int fd) const {
    return (std::find(_inviteList.begin(), _inviteList.end(), fd) != _inviteList.end());
}

const std::string& Channel::getName()           const { return _name; }
const std::string& Channel::getTopic()          const { return _topic; }
const std::string& Channel::getPassword()       const { return _password; }
const std::string& Channel::getCreationTime()   const { return _creationTime; }
int                Channel::getUserLimit()      const { return _userLimit; }
bool               Channel::isInviteOnly()      const { return _inviteOnly; }
bool               Channel::isTopicRestricted() const { return _topicRestricted; }
bool               Channel::hasPassword()       const { return _hasPassword; }
bool               Channel::hasUserLimit()      const { return _hasUserLimit; }

void Channel::setTopic(const std::string& t)    { _topic = t; }
void Channel::setPassword(const std::string& p) { _password = p; _hasPassword = true; }
void Channel::removePassword()                  { _password = ""; _hasPassword = false; }
void Channel::setUserLimit(int l)               { _userLimit = l; _hasUserLimit = true; }
void Channel::removeUserLimit()                 { _userLimit = 0; _hasUserLimit = false; }
void Channel::setInviteOnly(bool v)             { _inviteOnly = v; }
void Channel::setTopicRestricted(bool v)        { _topicRestricted = v; }