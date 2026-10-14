#include<string>
#include "SocialGraph.h"

class UserSystem
{
    public:
        UserSystem();
        bool registerUser(SocialGraph& graph);
        std::string loginUser();
};