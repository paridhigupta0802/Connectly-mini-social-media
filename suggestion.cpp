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