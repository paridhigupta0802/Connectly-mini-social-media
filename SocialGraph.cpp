#include "SocialGraph.h"

#include<iostream>

#include<jdbc/mysql_driver.h>
#include<jdbc/mysql_connection.h>
#include<jdbc/cppconn/prepared_statement.h>
#include<jdbc/cppconn/resultset.h>
#include<jdbc/cppconn/exception.h>

using namespace std;

SocialGraph::SocialGraph():people(nullptr){}

SocialGraph::~SocialGraph()
{
    clear();
}

PersonNode* SocialGraph::findPerson(const std::string& username)
{
    PersonNode* current=people;
    while(current != nullptr)
    {
        if(current->username == username)
            return current;
        current=current->next;
    }
    return nullptr;
}

void SocialGraph::addPerson(const std::string& username)
{
    if(username == "" || findPerson(username) != nullptr)
        return;

    PersonNode* person=new PersonNode;
    person->username=username;
    person->friends=nullptr;
    person->next=people;
    people=person;

    std::cout<<username<<" added to the social graph.\n";
}

void SocialGraph::addFriendEdge(const std::string& username1,const std::string& username2)
{
    if(username1 == username2)
        return;
    PersonNode* person1=findPerson(username1);
    PersonNode* person2=findPerson(username2);
    if(person1 == nullptr || person2 == nullptr)
    {
        return;
    }
    bool found1=false;
    bool found2=false;

    FriendNode* current=person1->friends;
    while(current != nullptr)
    {
        if(current->username == username2)
        {
            found1=true;
        }
        current=current->next;
    }
    current=person2->friends;
    while(current != nullptr)
    {
        if(current->username == username1)
        {
            found2=true;
        }
        current=current->next;
    }
    if(!found1)
    {
        FriendNode* node1=new FriendNode;
        node1->username=username2;
        node1->next=person1->friends;
        person1->friends=node1;
    }
    if(!found2)
    {
        FriendNode* node2=new FriendNode;
        node2->username=username1;
        node2->next=person2->friends;
        person2->friends=node2;
    }
    
}

bool SocialGraph::addFriendship(const std::string& username1,const std::string& username2)
{
    if(username1 == username2)
    {
        std::cout<<"You cannot add yourself as a friend.\n";
        return false;
    }
    
    PersonNode* person1=findPerson(username1);
    PersonNode* person2=findPerson(username2);

    if(person1 == nullptr || person2 == nullptr)
    {
        std::cout<<"Person does not exist.\n";
        return false;
    }
    FriendNode* current=person1->friends;
    while(current != nullptr)
    {
        if(current->username == username2)
        {
            std::cout<<"Friendship already exists.\n";
            return false;
        }
        current=current->next;
    }
    
    string first, second;
    if(username1 < username2)
    {
        first=username1;
        second=username2;
    }
    else
    {
        first=username2;
        second=username1;
    }
    
    sql::Connection* con=nullptr;
    sql::PreparedStatement* stmt=nullptr;

    try
    {
        sql::mysql::MySQL_Driver* driver=sql::mysql::get_mysql_driver_instance();
        con=driver->connect(dbHost,dbUser,dbPassword);
        con->setSchema("connectly");
        stmt=con->prepareStatement("INSERT INTO friendships (username,friend_username) VALUES (?, ?)");
        stmt->setString(1, first);        
        stmt->setString(2, second);
        stmt->executeUpdate();

        delete stmt;
        stmt=nullptr;

        delete con;
        con=nullptr;

        addFriendEdge(username1, username2);
        cout<<"Friendship saved successfully.\n";
        return true;
    }
    catch(sql::SQLException& e)
    {
        cerr<<"MySQL error saving friendship:"<<e.what()<<'\n';
        delete stmt;
        delete con;

        return false;
    }
}

bool SocialGraph::loadGraph(const std::string& host, const std::string& user, const std::string& password)
{
    clear();
    dbHost=host;
    dbUser=user;
    dbPassword=password;

    sql::Connection* con=nullptr;
    sql::PreparedStatement* usersStmt=nullptr;
    sql::ResultSet* usersResult=nullptr;
    sql::PreparedStatement* friendsStmt=nullptr;
    sql::ResultSet* friendsResult=nullptr;

    try
    {
        
        sql::mysql::MySQL_Driver* driver=sql::mysql::get_mysql_driver_instance();
        con=driver->connect(host,dbUser,dbPassword);
        con->setSchema("connectly");
        usersStmt=con->prepareStatement("SELECT username FROM users");
        usersResult=usersStmt->executeQuery();
        while(usersResult->next())
        {
            string username=usersResult->getString("username");
            addPerson(username);
        }
        delete usersResult;
        usersResult=nullptr;

        delete usersStmt;
        usersStmt=nullptr;

        friendsStmt=con->prepareStatement("SELECT username, friend_username FROM friendships");
        friendsResult=friendsStmt->executeQuery();
        while(friendsResult->next())
        {
            string username=friendsResult->getString("username");
            string friendUsername=friendsResult->getString("friend_username");
            addFriendEdge(username,friendUsername);
        }
        
        delete friendsResult;
        friendsResult=nullptr;

        delete friendsStmt;
        friendsStmt=nullptr;

        delete con;
        con=nullptr;

        cout<<"Users and friendships loaded successfully.\n";
        return true;
    }

    catch(sql::SQLException& e)
    {
        cerr<<"MySQL error loading graph: "<<e.what()<<'\n';

        delete friendsResult;
        delete friendsStmt;
        delete usersResult;
        delete usersStmt;
        delete con;

        clear();
        return false;

    }
}

void SocialGraph::clear()
{
    PersonNode* person=people;
    while(person != nullptr)
    {
        FriendNode* friendNode=person->friends;
        while(friendNode != nullptr)
        {
            FriendNode* nextfriend=friendNode->next;
            delete friendNode;
            friendNode=nextfriend;
        }

        PersonNode* nextPerson=person->next;
        delete person;
        person=nextPerson;
    }

    people=nullptr;
}