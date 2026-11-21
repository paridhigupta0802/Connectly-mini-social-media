#ifndef AUTH_H
#define AUTH_H

#include <string>
#include "SocialGraph.h"

class UserSystem
{
public:
    UserSystem();

    bool registerUser(
        SocialGraph& graph,
        const std::string& username,
        const std::string& password,
        const std::string& interests
    );

    std::string loginUser(
        const std::string& username,
        const std::string& password
    );
};

#endif
