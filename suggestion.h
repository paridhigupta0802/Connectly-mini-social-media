#ifndef FRIENDSUGGESTION_H
#define FRIENDSUGGESTION_H

#include "SocialGraph.h"
#include <string>
#include <vector>
#include <utility>

using namespace std;


// ================= SUGGESTION RESULT =================

struct SuggestionResult
{
    string username;
    int mutualCount;
    int interestCount;
    vector<string> sharedInterests;
};


// ================= VISITED LIST =================

class VisitedList
{
private:
    class VisitedNode
    {
    public:
        string username;
        VisitedNode* next;
    };

    VisitedNode* head;

public:
    VisitedList();

    bool isVisited(string nm);
    void addVisited(string nm);
    void clearVisited();

    ~VisitedList();
};


// ================= CANDIDATE LIST =================

class CandidateList
{
private:
    class CandidateNode
    {
    public:
        string username;
        int mutualCount;
        int interestCount;
        vector<string> sharedInterests;

        CandidateNode* next;
    };

    CandidateNode* head;

public:
    CandidateList();

    void addCandidate(
        const string& username,
        int mutualCount,
        const vector<string>& sharedInterests = {}
    );

    void sortCandidates();

    vector<SuggestionResult> getAllCandidates();

    void displayTopFive();
    void clearCandidates();

    ~CandidateList();
};


// ================= FRIEND SUGGESTION =================

class FriendSuggestion
{
private:
    int countMutualFriends(
        SocialGraph& graph,
        const string& username,
        const string& candidate
    );

public:
    vector<SuggestionResult> suggestTopK(
        SocialGraph& graph,
        const string& username
    );
};

#endif
