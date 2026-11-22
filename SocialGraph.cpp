#include "SocialGraph.h"

#include<iostream>
#include <windows.h>
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
    while(current!=nullptr)
    {
        if(current->username==username)
            return current;
        current=current->next;
    }
    return nullptr;
}

void SocialGraph::addPerson(const std::string& username)
{
    if(username==""||findPerson(username)!=nullptr)
        return;

    PersonNode* temp=new PersonNode(username);
    temp->next=people;
    people=temp;

    std::cout<<username<<" added to the social graph.\n";
}

void SocialGraph::addFriendEdge(const std::string& username1,const std::string& username2)
{
    if(username1==username2)
        return;
    PersonNode* person1=findPerson(username1);
    PersonNode* person2=findPerson(username2);
    if(person1==nullptr||person2==nullptr)
    {
        return;
    }
    FriendNode* temp=person1->friends;
    while(temp!=nullptr)
    {
        if(temp->username==username2)
        {
            return;
        }
        temp=temp->next;
    }
    FriendNode* node1=new FriendNode(username2);
    node1->next=person1->friends;
    person1->friends=node1;

   FriendNode* node2=new FriendNode(username1);
    node2->next=person2->friends;
    person2->friends=node2;
}
bool SocialGraph::addFriendship(const std::string& username1,const std::string& username2)
{
    if(username1==username2)
    {
        std::cout<<"You cannot add yourself as a friend.\n";
        return false;
    }
    
    PersonNode* person1=findPerson(username1);
    PersonNode* person2=findPerson(username2);

    if(person1==nullptr||person2==nullptr)
    {
        std::cout<<"Person does not exist.\n";
        return false;
    }
    FriendNode* current=person1->friends;
    while(current!=nullptr)
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
        stmt=nullptr;
        delete con;
        con=nullptr;

        return false;
    }
}


