#include "suggestion.h"
#include "Queue.h"
#include <iostream>
#include <string>
using namespace std;

struct VisitedNode
{
    string username;
    VisitedNode* next;
};

struct CandidateNode
{
    string username;
    int mutualCount;
    CandidateNode* next;
};


int countMutualFriends(SocialGraph& graph,const string& unm,const string& cnm)
{
    PersonNode* pll=graph.findPerson(unm);
    PersonNode* cpll=graph.findPerson(cnm);

    if (pll==nullptr||cpll==nullptr)
        return 0;

    int count=0;
    FriendNode* startFriend=pll->friends;
    while (startFriend!=nullptr)
    {
        FriendNode* candidateFriend=cpll->friends;
        while(candidateFriend!=nullptr)
        {
            if (startFriend->username==candidateFriend->username)
            {
                count++;
                break;
            }

            candidateFriend=candidateFriend->next;
        }
        startFriend = startFriend->next;
    }
    return count;
}

void addCandidate(CandidateNode*& head,const string& username,int mutualCount)
{
    CandidateNode* temp=new CandidateNode;
    temp->username=username;
    temp->mutualCount=mutualCount;
    temp->next=nullptr;
    if(head==nullptr)
    {
        head=temp;
        return;
    }
    CandidateNode* p=head;
    while (p->next != nullptr)
    {
        p=p->next;
    }
    p->next=temp;
}
