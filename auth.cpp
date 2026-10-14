#include "auth.h"

#include <iostream>
#include <string>

#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/exception.h>

using namespace std;

struct User
{
    string username;
    string password;
};


UserSystem::UserSystem(){}
bool UserSystem::registerUser(SocialGraph& graph)
{
    User u;
    cout << "\n===REGISTER===\n";
    cout << "Enter username: ";
    cin >> u.username;
    cout << "Enter password: ";
    cin >> u.password;

    if(u.username.empty() || u.password.empty())
    {
        cout<<"Username and password cannot be empty.\n";
        return false;
    }

    sql::Connection* con=nullptr;
    sql::PreparedStatement* checkStmt=nullptr;
    sql::ResultSet* result=nullptr;
    sql::PreparedStatement* insertStmt=nullptr;

    try
    {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();

        con = driver->connect("tcp://127.0.0.1:3306","root","root");
        con->setSchema("connectly");
        checkStmt =con->prepareStatement("SELECT username FROM users WHERE username = ?");
        checkStmt->setString(1, u.username);
        result = checkStmt->executeQuery();

        if (result->next())
        {
            cout << "Username already exists.\n";

            delete result;
            result=nullptr;
            delete checkStmt;
            checkStmt=nullptr;
            delete con;
            con=nullptr;

            return false;
        }

        delete result;
        result=nullptr;

        delete checkStmt;
        checkStmt=nullptr;
        insertStmt =con->prepareStatement("INSERT INTO users (username, password) VALUES (?, ?)");
        insertStmt->setString(1, u.username);
        insertStmt->setString(2, u.password);

        insertStmt->executeUpdate();

        delete insertStmt;
        insertStmt=nullptr;

        delete con;
        con=nullptr;

        graph.addPerson(u.username);
        cout<<"Registration Successful.\n";
        return true;
    }
    catch (sql::SQLException& e)
    {
        cout << "Database error: "<< e.what() << '\n';

        delete result;
        result=nullptr;
        delete checkStmt;
        checkStmt=nullptr;
        delete insertStmt;
        insertStmt=nullptr;
        delete con;
        con=nullptr;

        return false;
    }
}

string UserSystem::loginUser()
{
    string username, password;

    cout << "\n=== LOGIN ===\n";

    cout << "Enter username: ";
    cin >> username;

    cout << "Enter password: ";
    cin >> password;

    sql::Connection* con = nullptr;
    sql::PreparedStatement* stmt = nullptr;
    sql::ResultSet* res = nullptr;

    try
    {
        sql::mysql::MySQL_Driver* driver =sql::mysql::get_mysql_driver_instance();

        con = driver->connect("tcp://127.0.0.1:3306","root","root");

        con->setSchema("connectly");

        stmt = con->prepareStatement("SELECT username FROM users WHERE username = ? AND password = ?");

        stmt->setString(1, username);
        stmt->setString(2, password);

        res = stmt->executeQuery();

        if (res->next())
        {
            string loggedInUser = res->getString("username");

            cout << "Login successful! Welcome, "
                 << loggedInUser << "!\n";

            delete res;
            res=nullptr;
            delete stmt;
            stmt=nullptr;
            delete con;
            con=nullptr;

            return loggedInUser;
        }
        else
        {
            cout << "Invalid username or password.\n";
        }

        delete res;
        res=nullptr;
        delete stmt;
        stmt=nullptr;
        delete con;
        con=nullptr;

        return "";
    }
    catch (sql::SQLException& e)
    {
        cout << "Database error: " << e.what() << '\n';

        delete res;
        res=nullptr;
        delete stmt;
        stmt=nullptr;
        delete con;
        con=nullptr;

        return "";
    }
}