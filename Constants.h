#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <map>
#include <string>

class KeyPair
{
public:
    KeyPair(const std::string &vk = "",const std::string &name = "") : VKName(vk),
                                                                       Name(name)
                                                                       {}
    std::string VKName;
    std::string Name;
};


#endif // CONSTANTS_H
