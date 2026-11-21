#include "suggestion.h"
#include "Queue.h"

#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <memory>

#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/exception.h>

using namespace std;


// ================= INTEREST HELPER FUNCTIONS =================

// Remove spaces from the beginning and end of an interest.

string trimInterest(const string& interest)
{
    size_t start = interest.find_first_not_of(" \t\r\n");

    if (start == string::npos)
        return "";

    size_t end = interest.find_last_not_of(" \t\r\n");

    return interest.substr(start, end - start + 1);
}


// Convert interest to lowercase for comparison.

string normalizeInterest(const string& interest)
{
    string result = trimInterest(interest);

    transform(
        result.begin(),
        result.end(),
        result.begin(),
        [](unsigned char ch)
        {
            return static_cast<char>(tolower(ch));
        }
    );

    return result;
}


// Split comma-separated interests.

vector<string> splitInterests(const string& interests)
{
    vector<string> result;

    stringstream ss(interests);
    string interest;

    while (getline(ss, interest, ','))
    {
        interest = trimInterest(interest);

        if (!interest.empty())
            result.push_back(interest);
    }

    return result;
}


// Find common interests between two users.

vector<string> findSharedInterests(
    const vector<string>& user1,
    const vector<string>& user2)
{
    vector<string> shared;

    for (const string& interest1 : user1)
    {
        string normalized1 = normalizeInterest(interest1);

        for (const string& interest2 : user2)
        {
            if (normalized1 == normalizeInterest(interest2))
            {
                bool alreadyAdded = false;

                for (const string& existing : shared)
                {
                    if (normalizeInterest(existing) == normalized1)
                    {
                        alreadyAdded = true;
                        break;
                    }
                }

                if (!alreadyAdded)
                    shared.push_back(interest1);

                break;
            }
        }
    }

    return shared;
}


// ================= VISITED LIST =================

VisitedList::VisitedList()
{
    head = nullptr;
}

VisitedList::~VisitedList()
{
    clearVisited();
}

bool VisitedList::isVisited(string nm)
{
    VisitedNode* current = head;

    while (current != nullptr)
    {
        if (current->username == nm)
            return true;

        current = current->next;
    }

    return false;
}

void VisitedList::addVisited(string nm)
{
    VisitedNode* node = new VisitedNode;

    node->username = nm;
    node->next = head;

    head = node;
}

void VisitedList::clearVisited()
{
    while (head != nullptr)
    {
        VisitedNode* temp = head;

        head = head->next;

        delete temp;
    }
}


// ================= CANDIDATE LIST =================

CandidateList::CandidateList()
{
    head = nullptr;
}

CandidateList::~CandidateList()
{
    clearCandidates();
}

void CandidateList::clearCandidates()
{
    while (head != nullptr)
    {
        CandidateNode* temp = head;

        head = head->next;

        delete temp;
    }
}


// Add candidate and avoid duplicates.

void CandidateList::addCandidate(
    const string& username,
    int mutualCount,
    const vector<string>& sharedInterests)
{
    CandidateNode* current = head;

    // Check whether candidate already exists.

    while (current != nullptr)
    {
        if (current->username == username)
        {
            current->mutualCount = mutualCount;

            // Merge shared interests without duplicates.

            for (const string& interest : sharedInterests)
            {
                bool exists = false;

                for (const string& existing : current->sharedInterests)
                {
                    if (normalizeInterest(existing) ==
                        normalizeInterest(interest))
                    {
                        exists = true;
                        break;
                    }
                }

                if (!exists)
                    current->sharedInterests.push_back(interest);
            }

            current->interestCount =
                static_cast<int>(current->sharedInterests.size());

            return;
        }

        current = current->next;
    }

    // Candidate does not exist, so create a new node.

    CandidateNode* temp = new CandidateNode;

    temp->username = username;
    temp->mutualCount = mutualCount;
    temp->sharedInterests = sharedInterests;

    temp->interestCount =
        static_cast<int>(sharedInterests.size());

    temp->next = nullptr;

    if (head == nullptr)
    {
        head = temp;
        return;
    }

    CandidateNode* p = head;

    while (p->next != nullptr)
    {
        p = p->next;
    }

    p->next = temp;
}


// Sort candidates by combined score.

void CandidateList::sortCandidates()
{
    if (head == nullptr)
        return;

    for (CandidateNode* i = head; i != nullptr; i = i->next)
    {
        for (CandidateNode* j = i->next; j != nullptr; j = j->next)
        {
            int score1 = i->mutualCount + i->interestCount;
            int score2 = j->mutualCount + j->interestCount;

            if (score2 > score1)
            {
                swap(i->username, j->username);
                swap(i->mutualCount, j->mutualCount);
                swap(i->interestCount, j->interestCount);
                swap(i->sharedInterests, j->sharedInterests);
            }
        }
    }
}


// Return all candidates.

vector<SuggestionResult> CandidateList::getAllCandidates()
{
    vector<SuggestionResult> result;

    CandidateNode* current = head;

    while (current != nullptr)
    {
        SuggestionResult suggestion;

        suggestion.username = current->username;
        suggestion.mutualCount = current->mutualCount;
        suggestion.interestCount = current->interestCount;
        suggestion.sharedInterests = current->sharedInterests;

        result.push_back(suggestion);

        current = current->next;
    }

    return result;
}


// Display top five candidates.

void CandidateList::displayTopFive()
{
    int displayed = 0;

    CandidateNode* current = head;

    while (current != nullptr && displayed < 5)
    {
        cout << displayed + 1 << ". "
             << current->username << "\n";

        cout << "Mutual Friends: "
             << current->mutualCount << "\n";

        cout << "Shared Interests: ";

        if (current->sharedInterests.empty())
        {
            cout << "None";
        }
        else
        {
            for (size_t i = 0;
                 i < current->sharedInterests.size(); i++)
            {
                if (i > 0)
                    cout << ", ";

                cout << current->sharedInterests[i];
            }
        }

        cout << "\nScore: "
             << current->mutualCount + current->interestCount;

        cout << "\n-------------------------\n";

        displayed++;
        current = current->next;
    }

    if (displayed == 0)
    {
        cout << "No friend suggestions available.\n";
    }
}


// ================= MUTUAL FRIEND COUNT =================

int FriendSuggestion::countMutualFriends(
    SocialGraph& graph,
    const string& username,
    const string& candidate)
{
    PersonNode* person = graph.findPerson(username);

    PersonNode* candidatePerson = graph.findPerson(candidate);

    if (person == nullptr || candidatePerson == nullptr)
        return 0;

    int count = 0;

    FriendNode* startFriend = person->friends;

    while (startFriend != nullptr)
    {
        FriendNode* candidateFriend = candidatePerson->friends;

        while (candidateFriend != nullptr)
        {
            if (startFriend->username == candidateFriend->username)
            {
                count++;
                break;
            }

            candidateFriend = candidateFriend->next;
        }

        startFriend = startFriend->next;
    }

    return count;
}


// ================= FRIEND SUGGESTION BFS + INTERESTS =================

vector<SuggestionResult> FriendSuggestion::suggestTopK(
    SocialGraph& graph,
    const string& username)
{
    vector<SuggestionResult> recommendations;

    if (graph.findPerson(username) == nullptr)
        return recommendations;

    Queue q;

    VisitedList visited;

    CandidateList candidates;


    // =========================================
    // PART 1: EXISTING BFS LOGIC
    // =========================================

    // Start BFS from logged-in user.

    q.enqueue({ username, 0 });

    visited.addVisited(username);

    while (!q.isEmpty())
    {
        BFSNode current;

        if (!q.dequeue(current))
            break;

        // Distance 2 means friend-of-a-friend.

        if (current.distance == 2)
        {
            int mutualCount = countMutualFriends(
                graph,
                username,
                current.username
            );

            candidates.addCandidate(
                current.username,
                mutualCount
            );

            continue;
        }

        PersonNode* person = graph.findPerson(current.username);

        if (person == nullptr)
            continue;

        FriendNode* friendNode = person->friends;

        while (friendNode != nullptr)
        {
            string friendUsername = friendNode->username;

            if (!visited.isVisited(friendUsername))
            {
                visited.addVisited(friendUsername);

                q.enqueue({
                    friendUsername,
                    current.distance + 1
                });
            }

            friendNode = friendNode->next;
        }
    }


    // =========================================
    // PART 2: INTEREST-BASED SUGGESTIONS
    // =========================================

    try
    {
        sql::mysql::MySQL_Driver* driver =
            sql::mysql::get_mysql_driver_instance();

        unique_ptr<sql::Connection> con(
            driver->connect(
                graph.dbHost,
                graph.dbUser,
                graph.dbPassword
            )
        );

        con->setSchema("connectly");


        // Get logged-in user's interests.

        unique_ptr<sql::PreparedStatement> userStmt(
            con->prepareStatement(
                "SELECT interests FROM users WHERE username = ?"
            )
        );

        userStmt->setString(1, username);

        unique_ptr<sql::ResultSet> userResult(
            userStmt->executeQuery()
        );

        vector<string> userInterests;

        if (userResult->next() &&
            !userResult->isNull("interests"))
        {
            userInterests = splitInterests(
                userResult->getString("interests")
            );
        }

        userResult.reset();
        userStmt.reset();


        // Get all other users and their interests.

        unique_ptr<sql::PreparedStatement> allUsersStmt(
            con->prepareStatement(
                "SELECT username, interests "
                "FROM users WHERE username <> ?"
            )
        );

        allUsersStmt->setString(1, username);

        unique_ptr<sql::ResultSet> allUsersResult(
            allUsersStmt->executeQuery()
        );


        // Get direct friends to exclude them.

        PersonNode* currentUser = graph.findPerson(username);

        while (allUsersResult->next())
        {
            string candidateUsername =
                allUsersResult->getString("username");

            // Exclude direct friends.

            bool isDirectFriend = false;

            if (currentUser != nullptr)
            {
                FriendNode* friendNode = currentUser->friends;

                while (friendNode != nullptr)
                {
                    if (friendNode->username == candidateUsername)
                    {
                        isDirectFriend = true;
                        break;
                    }

                    friendNode = friendNode->next;
                }
            }

            if (isDirectFriend)
                continue;


            // Ignore users with NULL interests.

            if (allUsersResult->isNull("interests"))
                continue;


            vector<string> candidateInterests =
                splitInterests(
                    allUsersResult->getString("interests")
                );


            // Find common interests.

            vector<string> sharedInterests =
                findSharedInterests(
                    userInterests,
                    candidateInterests
                );


            // Add candidate only if at least one interest matches.

            if (!sharedInterests.empty())
            {
                int mutualCount = countMutualFriends(
                    graph,
                    username,
                    candidateUsername
                );

                candidates.addCandidate(
                    candidateUsername,
                    mutualCount,
                    sharedInterests
                );
            }
        }
    }
    catch (sql::SQLException& e)
    {
        cerr << "Database error while finding interest suggestions: "
             << e.what() << "\n";
    }


    // =========================================
    // PART 3: SORT AND RETURN TOP FIVE
    // =========================================

    candidates.sortCandidates();

    vector<SuggestionResult> allCandidates =
        candidates.getAllCandidates();

    int count = 0;

    for (const SuggestionResult& candidate : allCandidates)
    {
        if (count == 5)
            break;

        recommendations.push_back(candidate);

        count++;
    }

    return recommendations;
}
