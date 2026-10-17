#ifndef FRIENDSUGGESTION_H
#define FRIENDSUGGESTION_H

#include "SocialGraph.h"
#include <string>

class FriendSuggestion
{
public:
    void suggestTopK(SocialGraph& graph,const std::string& username,int k);
};

#endif
