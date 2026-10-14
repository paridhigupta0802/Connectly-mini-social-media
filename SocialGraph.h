#ifndef SOCIALGRAPH_H
#define SOCIALGRAPH_H

#include<string>
struct FriendNode
{
    std::string username;
    FriendNode* next;
};
struct PersonNode
{
    std::string username;
    FriendNode* friends;
    PersonNode* next;
};

class SocialGraph
{
    private:
        PersonNode* people;
        std::string dbHost;
        std::string dbUser;
        std::string dbPassword;

        void addFriendEdge(const std::string& username1, const std::string& username2);
    public:
        SocialGraph();
        ~SocialGraph();

        PersonNode* findPerson(const std::string& username);
        void addPerson(const std::string& username);
        bool addFriendship(const std::string& username1, const std::string& username2);
        bool loadGraph(const std::string& host, const std::string& dbUser, const std::string& dbPassword);
        void clear();               
};

#endif