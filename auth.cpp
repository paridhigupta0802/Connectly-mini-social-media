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

class UserSystem
{
public:
 UserSystem();
    bool registerUser();
    string loginUser();
};

UserSystem::UserSystem(){}
bool UserSystem::registerUser()
{
    User u;
    cout << "\n===REGISTER===\n";
    cout << "Enter username: ";
    cin >> u.username;
    cout << "Enter password: ";
    cin >> u.password;
    try
    {
        sql::mysql::MySQL_Driver* driver;
        sql::Connection* con;

        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect(
            "tcp://127.0.0.1:3306",
            "root",
            "root"
        );

        con->setSchema("connectly");
        sql::PreparedStatement* checkStmt =
            con->prepareStatement(
                "SELECT username FROM users WHERE username = ?"
            );

        checkStmt->setString(1, u.username);

        sql::ResultSet* result = checkStmt->executeQuery();

        if (result->next())
        {
            cout << "Username already exists.\n";

            delete result;
            delete checkStmt;
            delete con;

            return false;
        }

        delete result;
        delete checkStmt;
        sql::PreparedStatement* insertStmt =
            con->prepareStatement(
                "INSERT INTO users (username, password) "
                "VALUES (?, ?)"
            );

        insertStmt->setString(1, u.username);
        insertStmt->setString(2, u.password);

        insertStmt->executeUpdate();

        cout << "Registration successful!\n";

        delete insertStmt;
        delete con;

        return true;
    }
    catch (sql::SQLException& e)
    {
        cout << "Database error: "
             << e.what() << '\n';

        return false;
    }
}

string UserSystem::loginUser()
{
    User u;

    cout << "\n=== LOGIN ===\n";

    cout << "Enter username: ";
    cin >> u.username;

    cout << "Enter password: ";
    cin >> u.password;

    try
    {
        sql::mysql::MySQL_Driver* driver;
        sql::Connection* con;

        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect(
            "tcp://127.0.0.1:3306",
            "root",
            "root"
        );

        con->setSchema("connectly");

        sql::PreparedStatement* stmt =
            con->prepareStatement(
                "SELECT username FROM users "
                "WHERE username = ? AND password = ?"
            );

        stmt->setString(1, u.username);
        stmt->setString(2, u.password);

        sql::ResultSet* result = stmt->executeQuery();

        if (result->next())
        {
            cout << "Login successful!\n";

            string username = result->getString("username");

            delete result;
            delete stmt;
            delete con;

            return username;
        }

        cout << "Invalid username or password.\n";

        delete result;
        delete stmt;
        delete con;

        return "";
    }
    catch (sql::SQLException& e)
    {
        cout << "Database error: "
             << e.what() << '\n';

        return "";
    }
}
