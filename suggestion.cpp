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

void clearCandidates(CandidateNode*& head)
{
    while (head != nullptr)
    {
        CandidateNode* temp = head;
        head = head->next;
        delete temp;
    }
}

void FriendSuggestion::suggestTopK(SocialGraph& graph,const string& nm,int 5)
{
    if (graph.findPerson(nm)==nullptr)
    {
        cout<<"User does not exist in the social graph.\n";
        return;
    }

    Queue q;
    VisitedNode* v= nullptr;
    CandidateNode* c = nullptr;

  
    q.enqueue({nm,0});
    addVisited(v,nm);

    cout << "\n===== FRIEND SUGGESTIONS =====\n";
    cout << "User: " << nm << "\n\n";

  
    while(!q.isEmpty())
    {
        BFSNode current;
        if (!q.dequeue(current))
            break;

        if (current.distance == 2)
        {
            int mutualCount=countMutualFriends(graph,nm,current.username);
            addCandidate(c,current.username,mutualCount);
            continue;
        }

        PersonNode* person=graph.findPerson(current.username);
        if (person==nullptr)
            continue;
        FriendNode* friendNode=person->friends;

        while(friendNode!=nullptr)
        {
            string friendUsername = friendNode->username;
            if (!isVisited(v,friendUsername))
            {
                addVisited(v,friendUsername);
                q.enqueue({friendUsername,current.distance + 1});
            }
            friendNode=friendNode->next;
        }
    }

    sortCandidates(c);

    int displayed = 0;
    CandidateNode* current=c;

    while(current!=nullptr&& displayed < 5)
    {
        cout<<displayed+1<< ". "<<current->username<<" (Mutual friends: "<<current->mutualCount<< ")\n";
        displayed++;
        current=current->next;
    }

    if (displayed==0)
    {
        cout<<"No friend suggestions available.\n";
    }

    clearVisited(v);
    clearCandidates(c);
}
