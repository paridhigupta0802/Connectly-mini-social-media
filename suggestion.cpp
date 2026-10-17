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

