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
bool SocialGraph::deleteFriendship(const std::string& username1,const std::string& username2)
{
    if(username1 == username2)
    {
        std::cout<<"Cannot delete friendship with yourself.\n";
        return false;
    }

    if(!searchPerson(username1) || !searchPerson(username2) )
    {
        cout<<"Person does not exist.\n";
        return false;
    }
    
    string first=username1;
    string second=username2;
    if(first > second)
    {
        string temp=first;
        first=second;
        second=temp;    
    }
    
    sql::Connection* con=nullptr;
    sql::PreparedStatement* pstmt=nullptr;

    try
    {
        sql::mysql::MySQL_Driver* driver=sql::mysql::get_mysql_driver_instance();
        con=driver->connect(dbHost,dbUser,dbPassword);
        con->setSchema("connectly");
        pstmt=con->prepareStatement("DELETE FROM friendships " 
            "WHERE username = ? AND friend_username = ?" );
        pstmt->setString(1, first);        
        pstmt->setString(2, second);

        int rows=pstmt->executeUpdate();

        delete pstmt;
        pstmt=nullptr;

        delete con;
        con=nullptr;

        if(rows == 0)
        {
            cout<<"Friendships not found.\n";
            return false;
        }

        PersonNode* person1=findPerson(username1);
        PersonNode* person2=findPerson(username2);

        if(person1 != nullptr )
        {
            FriendNode* current=person1->friends;
            FriendNode* previous=nullptr;
            while(current != nullptr)
            {
                if(current->username == username2)
                {
                    if(previous == nullptr)
                    {
                        person1->friends=current->next;
                    }
                    else
                    {
                        previous->next=current->next;
                    }
                    delete current;
                    break;
                }
                previous=current;
                current=current->next;
            }
        }

         if(person2 != nullptr )
        {
            FriendNode* current=person2->friends;
            FriendNode* previous=nullptr;
            while(current != nullptr)
            {
                if(current->username == username1)
                {
                    if(previous == nullptr)
                    {
                        person2->friends=current->next;
                    }
                    else
                    {
                        previous->next=current->next;
                    }
                    delete current;
                    break;
                }
                previous=current;
                current=current->next;
            }
        }
        cout<<"Friendship deleted successfully.\n";
        return true;

    }
    catch(sql::SQLException& e)
    {
        cerr<<"MySQL error :"<<e.what()<<'\n';
        delete pstmt;
        pstmt=nullptr;
        delete con;
        con=nullptr;

        return false;
    }
}

void SocialGraph::displayFriends(const string& username)
{
    sql::Connection* con = nullptr;
    sql::PreparedStatement* pstmt = nullptr;
    sql::ResultSet* res = nullptr;

    try
    {
        sql::mysql::MySQL_Driver* driver =sql::mysql::get_mysql_driver_instance();

        con = driver->connect(dbHost, dbUser, dbPassword);
        con->setSchema("connectly");

        pstmt = con->prepareStatement("SELECT username FROM users WHERE username = ?");
        pstmt->setString(1, username);
        res = pstmt->executeQuery();

        bool userExists = res->next();

        delete res;
        res = nullptr;

        delete pstmt;
        pstmt = nullptr;

        if (!userExists)
        {
            cout << "Person does not exist.\n";
            delete con;
            con = nullptr;
            return;
        }

        pstmt = con->prepareStatement("SELECT CASE WHEN username = ? "
            "THEN friend_username ELSE username END AS friend_name "
            "FROM friendships "
            "WHERE username = ? OR friend_username = ?");
        pstmt->setString(1, username);
        pstmt->setString(2, username);
        pstmt->setString(3, username);

        res = pstmt->executeQuery();

        cout << "\nFriends of " << username << ":\n";

        bool found = false;

        while (res->next())
        {
            cout << res->getString("friend_name") << '\n';
            found = true;
        }

        if (!found)
        {
            cout << "No friends found.\n";
        }

        delete res;
        res=nullptr;
        delete pstmt;
        pstmt=nullptr;
        delete con;
        con=nullptr;
    }
    catch (sql::SQLException& e)
    {
        cerr << "MySQL error: " << e.what() << '\n';
        delete res;
        res=nullptr;
        delete pstmt;
        pstmt=nullptr;
        delete con;
        con=nullptr;
    }
}

bool SocialGraph::searchPerson(const string& username)
{
    sql::Connection* con=nullptr;
    sql::PreparedStatement* pstmt=nullptr;
    sql::ResultSet* res=nullptr;

    try
    {
        sql::mysql::MySQL_Driver* driver=sql::mysql::get_mysql_driver_instance();
        con=driver->connect(dbHost,dbUser,dbPassword);
        con->setSchema("connectly");
        pstmt=con->prepareStatement("SELECT username FROM users WHERE username = ?");
        pstmt->setString(1, username);        
        res=pstmt->executeQuery();
        bool found=res->next();

        delete res;
        res=nullptr;
        delete pstmt;
        pstmt=nullptr;
        delete con;
        con=nullptr;

        return found;
    }

    catch(sql::SQLException& e)
    {
        cerr<<"MySQL error:"<<e.what()<<'\n';
        
        delete res;
        res=nullptr;
        delete pstmt;
        pstmt=nullptr;
        delete con;
        con=nullptr;

        return false;

    }  
}
       
bool SocialGraph::deleteAccount(const string& username)
{
    sql::Connection* con = nullptr;
    sql::PreparedStatement* pstmt = nullptr;
    sql::ResultSet* res = nullptr;

    try
    {
        sql::mysql::MySQL_Driver* driver =sql::mysql::get_mysql_driver_instance();

        con = driver->connect(dbHost, dbUser, dbPassword);
        con->setSchema("connectly");

        pstmt = con->prepareStatement("SELECT username FROM users WHERE username = ?");

        pstmt->setString(1, username);
        res = pstmt->executeQuery();

        if (!res->next())
        {
            cout << "Account does not exist.\n";

            delete res;
            res=nullptr;
            delete pstmt;
            pstmt=nullptr;
            delete con;
            con=nullptr;

            return false;
        }

        delete res;
        res = nullptr;

        delete pstmt;
        pstmt = nullptr;

        pstmt = con->prepareStatement("DELETE FROM friendships "
            "WHERE username = ? OR friend_username = ?");

        pstmt->setString(1, username);
        pstmt->setString(2, username);
        pstmt->executeUpdate();

        delete pstmt;
        pstmt = nullptr;

        pstmt = con->prepareStatement("DELETE FROM users WHERE username = ?");

        pstmt->setString(1, username);

        int rows = pstmt->executeUpdate();

        delete pstmt;
        pstmt = nullptr;

        delete con;
        con = nullptr;

        if (rows > 0)
        {
            if(loadGraph(dbHost, dbUser, dbPassword))
            {
            cout << "Account deleted successfully.\n";
            return true;
            }
            cout << "Account deleted ,but graph could noot be refreshed.\n";
            return false;
        }

        cout << "Account deletion failed.\n";
        return false;
    }
    catch (sql::SQLException& e)
    {
        cerr << "MySQL error: " << e.what() << '\n';

        delete res;
        res=nullptr;
        delete pstmt;
        pstmt=nullptr;
        delete con;
        con=nullptr;

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
        MessageBoxA(
    nullptr,
    host.c_str(),
    "Hostname received by loadGraph",
    MB_OK
);
con=driver->connect(host,dbUser,dbPassword);
        
        con->setSchema("connectly");
        usersStmt=con->prepareStatement("SELECT * FROM users ORDER BY username DESC");
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
    MessageBoxA(
        nullptr,
        e.what(),
        "Actual MySQL Error",
        MB_OK | MB_ICONERROR
    );

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

void displayGraph(const SocialGraph&  graph)
{
    PersonNode* person=graph.people;
    if(person == nullptr)
    {
        cout<<"Social Graph is Empty.\n";
        return;
    }
    cout<<"\n=====Connectly Social Graph=====\n";
    while(person != nullptr)
    {
        cout<<person->username<<" -> ";
        FriendNode* current=person->friends;
        if(current == nullptr)
        {
            cout<<"No friends";
        }
        while(current != nullptr)
        {
            cout<<current->username;
            if(current->next != nullptr)
            {
                cout<<" -> ";
            }
            current=current->next;
        }
        cout<<'\n';
        person=person->next;
    }
}






std::vector<std::string> SocialGraph::getFriends(
    const std::string& username)
{
    std::vector<std::string> friends;

    sql::Connection* con = nullptr;
    sql::PreparedStatement* stmt = nullptr;
    sql::ResultSet* res = nullptr;

    try
    {
        sql::mysql::MySQL_Driver* driver =
            sql::mysql::get_mysql_driver_instance();

        con = driver->connect(dbHost, dbUser, dbPassword);
        con->setSchema("connectly");

        stmt = con->prepareStatement(
            "SELECT CASE "
            "WHEN username = ? THEN friend_username "
            "ELSE username END AS friend_name "
            "FROM friendships "
            "WHERE username = ? OR friend_username = ? "
            "ORDER BY friend_name"
        );

        stmt->setString(1, username);
        stmt->setString(2, username);
        stmt->setString(3, username);

        res = stmt->executeQuery();

        while (res->next())
        {
            friends.push_back(
                res->getString("friend_name")
            );
        }

        delete res;
        delete stmt;
        delete con;

        return friends;
    }
    catch (sql::SQLException& e)
    {
        delete res;
        delete stmt;
        delete con;

        std::cerr << "Error loading friends: "
                  << e.what() << '\n';

        return {};
    }
}

