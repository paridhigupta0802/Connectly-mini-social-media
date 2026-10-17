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
bool isVisited(VisitedNode* head,string nm)
{
    while (head!= nullptr)
    {
        if (head->username == nm)
            return true;
        head=head->next;
    }
    return false;
}

void addVisited(VisitedNode* &head,string nm)
{
    VisitedNode* node=new VisitedNode;
    node->username=nm;
    node->next=head;
    head=node;
}

void clearVisited(VisitedNode*& head)
{
    while(head!=nullptr)
    {
        VisitedNode* temp =head;
        head=head->next;
        delete temp;
    }
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

void sortCandidates(CandidateNode*& head)
{
    if (head==nullptr)
        return;

    for (CandidateNode* i= head;i!=nullptr;i=i->next)
    {
        for (CandidateNode* j=i->next;j!=nullptr;j=j->next)
        {
            if (j->mutualCount>i->mutualCount)
            {
                string tempUsername = i->username;
                i->username = j->username;
                j->username = tempUsername;

                int tempCount = i->mutualCount;
                i->mutualCount = j->mutualCount;
                j->mutualCount = tempCount;
            }
        }
    }
}

void clearVisited(VisitedNode*& head)
{
    while(head!=nullptr)
    {
        VisitedNode* temp =head;
        head=head->next;
        delete temp;
    }
}

void addVisited(VisitedNode* &head,string nm)
{
    VisitedNode* node=new VisitedNode;
    node->username=nm;
    node->next=head;
    head=node;
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

void sortCandidates(CandidateNode*& head)
{
    if (head==nullptr)
        return;

    for (CandidateNode* i= head;i!=nullptr;i=i->next)
    {
        for (CandidateNode* j=i->next;j!=nullptr;j=j->next)
        {
            if (j->mutualCount>i->mutualCount)
            {
                string tempUsername = i->username;
                i->username = j->username;
                j->username = tempUsername;

                int tempCount = i->mutualCount;
                i->mutualCount = j->mutualCount;
                j->mutualCount = tempCount;
            }
        }
    }
}

void FriendSuggestion::suggestTopK(SocialGraph& graph,const string& nm,int k)
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

    while(current!=nullptr&& displayed < k)
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
