#ifndef SOCIALGRAPH_H
#define SOCIALGRAPH_H

#include<string>
#include <vector>

class SocialGraph;
class FriendSuggestion;

class FriendNode
{
    private:
        std::string username;
        FriendNode* next;

    public:
        FriendNode(const std::string& name)
        {
            username=name;
            next=nullptr;
        }

        friend class SocialGraph;
        friend class FriendSuggestion;
        friend void displayGraph(const SocialGraph& graph);
};

class PersonNode
{
    private:
        std::string username;
        FriendNode* friends;
        PersonNode* next;

    public:
        PersonNode(const std::string& name)
        {
            username=name;
            friends=nullptr;
            next=nullptr;
        }

        friend class SocialGraph;
        friend class FriendSuggestion;
        friend void displayGraph(const SocialGraph& graph);
};

class SocialGraph
{
    private:
        PersonNode* people;
        std::string dbHost;
        std::string dbUser;
        std::string dbPassword;

        void addFriendEdge(
            const std::string& username1,
            const std::string& username2
        );

        // ADDED: Allows FriendSuggestion to access database credentials.
        friend class FriendSuggestion;

    public:
        SocialGraph();
        ~SocialGraph();

        PersonNode* findPerson(const std::string& username);
        void addPerson(const std::string& username);

        bool addFriendship(
            const std::string& username1,
            const std::string& username2
        );

        bool deleteFriendship(
            const std::string& username1,
            const std::string& username2
        );

        bool sendFriendRequest(
            const std::string& from,
            const std::string& to
        );

        bool acceptFriendRequest(
            const std::string& acceptor,
            const std::string& requester
        );

        bool removeFriendRequest(
            const std::string& from,
            const std::string& to
        );

        std::vector<std::string> getFriendRequests(
            const std::string& username,
            bool incoming
        );

        std::string lastRequestError;

        bool deleteAccount(const std::string& username);
        void displayFriends(const std::string& username);

        friend void displayGraph(const SocialGraph& graph);

        bool searchPerson(const std::string& username);

        bool loadGraph(
            const std::string& host,
            const std::string& dbUser,
            const std::string& dbPassword
        );

        void clear();

        std::vector<std::string> getFriends(
            const std::string& username
        );
};

#endif
