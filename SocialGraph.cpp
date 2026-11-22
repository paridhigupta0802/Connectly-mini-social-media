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
