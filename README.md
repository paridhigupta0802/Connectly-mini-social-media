# Connectly-mini-social-media

## About the Project
Connectly is a C++-based social media application that uses MySQL to store user information and friendships. It demonstrates how users and their connections can be managed using a database and a social graph.

## Features
- **User Registration:** Allows new users to create an account.
- **User Login:** Allows registered users to log in.
- **Friendship Management:** Supports adding friendships between users.
- **Social Graph:** Represents users and their friendships using a graph-like structure.
- **MySQL Database Integration:** Stores and retrieves user and friendship information.
- **Friend Display and Removal:** Intended to allow users to view and remove friends as these features are implemented.

## Technologies Used
- **Programming Language:** C++
- **Database:** MySQL
- **Database Connectivity:** MySQL Connector/C++
- **Version Control:** Git and GitHub

## Database
The project uses a MySQL database named `connectly`.

The database includes:
- A `users` table for storing user information.
- A `friendships` table for storing connections between users.

Each friendship is stored using usernames, with database constraints to prevent duplicate connections and invalid self-friendships.

## Project Setup
1. Install a C++ compiler.
2. Install MySQL Server and create the `connectly` database.
3. Install and configure MySQL Connector/C++.
4. Set up the required database tables.
5. Configure the database connection details in the project.
6. Compile and run the project using the build instructions provided by the team.

## Contributors
This project is developed collaboratively as a team project.

## Project Status
The project is under development. Features and documentation may be updated as development progresses.
