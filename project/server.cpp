#include <iostream>
#include "server.h"

using namespace std;

#define PORT 3000
#define BUFFER_SIZE 1024

sqlite3 *db;

void log_in(int client_socket, const string &username, const string &password)
{
    sqlite3_stmt *stmt;
    string query = "SELECT username, password FROM Users WHERE username = ? AND password = ?";
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        cerr << "Error preparing query: " << sqlite3_errmsg(db) << endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        cout << "Login successful" << endl;
        send(client_socket, "Y_login", strlen("Y_login"), 0);
        functions(client_socket);
    }
    else
    {
        cout << "Login failed" << endl;
        send(client_socket, "N_login", strlen("N_login"), 0);
    }

    sqlite3_finalize(stmt);
}

void register_user(int client_socket, const string &username, const string &password)
{
    sqlite3_stmt *stmt;
    string query = "SELECT username FROM Users WHERE username = ?";
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        cerr << "Error preparing query: " << sqlite3_errmsg(db) << endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        cout << "Username existed" << endl;
        send(client_socket, "N_register", strlen("N_register"), 0);
    }
    else
    {
        cout << "Registration accepted" << endl;
        send(client_socket, "Y_register", strlen("Y_register"), 0);

        query = "INSERT INTO Users (username, password) VALUES (?, ?)";
        if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            cerr << "Error preparing query: " << sqlite3_errmsg(db) << endl;
            return;
        }

        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            cerr << "Error inserting user data: " << sqlite3_errmsg(db) << endl;
        }

        sqlite3_finalize(stmt);
    }
}

void functions(int client_socket)
{
    char buffer[BUFFER_SIZE];
    int bytes_received;

    while (true)
    {
        bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0)
        {
            break;
        }
        buffer[bytes_received] = '\0';

        string received(buffer);

        if (received == "exit")
        {
            break;
        }

        vector<string> type1 = split(received, ' ');

        if (type1.size() < 2 || (type1[0].compare("search") != 0 && type1[0].compare("book") != 0 && type1[0].compare("manage") != 0))
        {
            cout << "Invalid format" << endl;
            send(client_socket, "N_in", strlen("N_in"), 0);
            continue;
        }

        if (type1[0] == "search")
        {
            vector<string> search_params = split(type1[1], ',');

            if (search_params.size() == 2)
            {
                string departure_point = search_params[0];
                string destination_point = search_params[1];

                search_flight(client_socket, departure_point, destination_point);
            }
            else
            {
                string error_message = "Missing element!";
                cout << error_message << endl;
                send(client_socket, "N_search", strlen("N_search"), 0);
            }
        }
        else if (type1[0] == "book")
        {
            vector<string> book_params = split(type1[1], ',');
            if (book_params.size() >= 3)
            {
                string flight_number = book_params[0];
                string seat_number = book_params[1];

                string confirmation_msg = "Booking successful!";
                send(client_socket, confirmation_msg.c_str(), confirmation_msg.length(), 0);
            }
            else
            {
                string error_message = "Invalid format for booking. Please provide necessary details.";
                cout << error_message << endl;
                send(client_socket, "N_book", strlen("N_book"), 0);
            }
        }
        else if (type1[0] == "manage")
        {
            vector<string> manage_params = split(type1[1], ',');

            // Implement manage functionality here
        }
    }

    close(client_socket);
}

void search_flight(int client_socket, const string &departure_point, const string &destination_point)
{
    try
    {
        sqlite3_stmt *stmt;
        string query = "SELECT * FROM Flights WHERE departure_point = ? AND destination_point = ?";
        if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            cerr << "Error preparing query: " << sqlite3_errmsg(db) << endl;
            send(client_socket, "N_found", strlen("N_found"), 0);
            return;
        }

        sqlite3_bind_text(stmt, 1, departure_point.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, destination_point.c_str(), -1, SQLITE_STATIC);

        string result_str = "Y_found/";

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            result_str += to_string(sqlite3_column_int(stmt, 0)) + ",";
            result_str += to_string(sqlite3_column_int(stmt, 1)) + ",";
            // Add other columns here
            result_str += ";";
        }

        if (result_str == "Y_found/")
        {
            send(client_socket, "N_found", strlen("N_found"), 0);
        }
        else
        {
            send(client_socket, result_str.c_str(), result_str.length(), 0);
        }

        sqlite3_finalize(stmt);
    }
    catch (exception &e)
    {
        cerr << "Error occurred during flight search: " << e.what() << endl;
        send(client_socket, "N_found", strlen("N_found"), 0);
    }
}

void connect_client(int client_socket)
{
    char buffer[BUFFER_SIZE];
    int bytes_received;

    cout << "Connected to client" << endl;

    while (true)
    {
        bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0)
        {
            break;
        }
        buffer[bytes_received] = '\0';

        string received(buffer);

        if (received == "exit")
        {
            break;
        }

        size_t space_pos = received.find(' ');
        if (space_pos == string::npos)
        {
            cout << "Invalid format" << endl;
            send(client_socket, "N_format", strlen("N_format"), 0);
            continue;
        }

        string command = received.substr(0, space_pos);
        string args = received.substr(space_pos + 1);

        if (command == "login")
        {
            size_t comma_pos = args.find(',');
            if (comma_pos == string::npos)
            {
                cout << "Invalid format" << endl;
                send(client_socket, "N_format", strlen("N_format"), 0);
                continue;
            }

            string username = args.substr(0, comma_pos);
            string password = args.substr(comma_pos + 1);

            cout << "Login requested" << endl;
            log_in(client_socket, username, password);
        }
        else if (command == "register")
        {
            size_t comma_pos = args.find(',');
            if (comma_pos == string::npos)
            {
                cout << "Invalid format" << endl;
                send(client_socket, "N_format", strlen("N_format"), 0);
                continue;
            }

            string username = args.substr(0, comma_pos);
            string password = args.substr(comma_pos + 1);

            cout << "Registration requested" << endl;
            register_user(client_socket, username, password);
        }
    }

    cout << "Connection closed" << endl;
    close(client_socket);
}

int main()
{
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    if (sqlite3_open("flight_database.db", &db) != SQLITE_OK)
    {
        cerr << "Error opening database" << endl;
        return 1;
    }

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        cerr << "Error creating server socket" << endl;
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        cerr << "Error binding server socket" << endl;
        return 1;
    }

    if (listen(server_socket, SOMAXCONN) == -1)
    {
        cerr << "Error listening on server socket" << endl;
        return 1;
    }

    cout << "Server listening on port " << PORT << "..." << endl;

    while (true)
    {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket == -1)
        {
            cerr << "Error accepting client connection" << endl;
            continue;
        }

        thread client_thread(connect_client, client_socket);
        client_thread.detach();
    }

    close(server_socket);
    sqlite3_close(db);

    return 0;
}
