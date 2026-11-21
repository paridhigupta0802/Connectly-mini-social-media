#include "auth.h"

#include <cctype>
#include <string>
#include <memory>

#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/exception.h>

using namespace std;

// ================= PASSWORD HASHING =================

string hashPassword(string password)
{
    long long hash = 0;
    int position = 1;

    for (unsigned char ch : password)
    {
        hash = (hash * 31 + ch + position * position)
               % 1000000007;

        position++;
    }
    return to_string(hash);
}


// ================= USER SYSTEM =================

UserSystem::UserSystem() {}


// ================= PASSWORD VALIDATION =================

bool isValidPassword(const string& password)
{
    if (password.length() < 8)
        return false;

    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;
    bool hasSpecial = false;

    for (char ch : password)
    {
        if (isupper(static_cast<unsigned char>(ch)))
            hasUpper = true;

        else if (islower(static_cast<unsigned char>(ch)))
            hasLower = true;

        else if (isdigit(static_cast<unsigned char>(ch)))
            hasDigit = true;

        else
            hasSpecial = true;
    }
    return hasUpper && hasLower && hasDigit && hasSpecial;
}


// ================= REGISTRATION =================

bool UserSystem::registerUser(
    SocialGraph& graph,
    const string& username,
    const string& password,
    const string& interests)
{
    // Check empty inputs
    if (username.empty() || password.empty())
        return false;

    // Username cannot contain spaces
    if (username.find_first_of(" \t\n") != string::npos)
        return false;

    // Validate password
    if (!isValidPassword(password))
        return false;

    // Hash password
    string hashedPassword = hashPassword(password);

    try
    {
        sql::mysql::MySQL_Driver* driver =
            sql::mysql::get_mysql_driver_instance();

        unique_ptr<sql::Connection> con(
            driver->connect(
                "tcp://127.0.0.1:3306",
                "root",
                "root"
            )
        );

        con->setSchema("connectly");

        // Check whether username already exists
        unique_ptr<sql::PreparedStatement> checkStmt(
            con->prepareStatement(
                "SELECT username FROM users WHERE username = ?"
            )
        );

        checkStmt->setString(1, username);

        unique_ptr<sql::ResultSet> result(
            checkStmt->executeQuery()
        );

        if (result->next())
        {
            return false;
        }

        result.reset();
        checkStmt.reset();

        // Insert new user along with interests
        unique_ptr<sql::PreparedStatement> insertStmt(
            con->prepareStatement(
                "INSERT INTO users (username, password, interests) "
                "VALUES (?, ?, ?)"
            )
        );

        insertStmt->setString(1, username);
        insertStmt->setString(2, hashedPassword);
        insertStmt->setString(3, interests);

        insertStmt->executeUpdate();
        // Add user to in-memory social graph
        graph.addPerson(username);

        return true;
    }
    catch (sql::SQLException& e)
    {
        return false;
    }
}


// ================= LOGIN =================

string UserSystem::loginUser(
    const string& username,
    const string& password)
{
    if (username.empty() || password.empty())
        return "";

    // Hash entered password
    string hashedPassword = hashPassword(password);

    try
    {
        sql::mysql::MySQL_Driver* driver =
            sql::mysql::get_mysql_driver_instance();

        unique_ptr<sql::Connection> con(
            driver->connect(
                "tcp://127.0.0.1:3306",
                "root",
                "root"
            )
        );

        con->setSchema("connectly");

        // Verify username and password
        unique_ptr<sql::PreparedStatement> stmt(
            con->prepareStatement(
                "SELECT username FROM users "
                "WHERE username = ? AND password = ?"
            )
        );

        stmt->setString(1, username);
        stmt->setString(2, hashedPassword);

        unique_ptr<sql::ResultSet> res(
            stmt->executeQuery()
        );

        if (res->next())
        {
            return res->getString("username");
        }

        return "";
    }
    catch (sql::SQLException& e)
    {
        return "";
    }
}
