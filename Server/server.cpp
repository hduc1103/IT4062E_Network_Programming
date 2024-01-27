#include "server.h"

sqlite3 *db;

int main()
{
    if (sqlite3_config(SQLITE_CONFIG_MULTITHREAD) != SQLITE_OK)
    {
        cerr << "Failed to set SQLite to multi-threaded mode." << endl;
        return 1;
    }
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

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
        close(server_socket);
        return 1;
    }

    if (listen(server_socket, SOMAXCONN) == -1)
    {
        cerr << "Error listening on server socket" << endl;
        close(server_socket);
        return 1;
    }

    std::cout << "Server listening on port " << PORT << "..." << endl;

    while (true)
    {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket == -1)
        {
            cerr << "Error accepting client connection" << endl;
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN) == NULL)
        {
            cerr << "Error converting IP address" << endl;
            close(client_socket);
            continue;
        }
        std::cout << "Received request from " << client_ip << endl;
        thread client_thread(connect_client, client_socket);
        client_thread.detach();
    }

    close(server_socket);
    return 0;
}
void connect_client(int client_socket)
{
    char buffer[BUFFER_SIZE];
    int bytes_received;

    if (sqlite3_open("Server/flight_database.db", &db) != SQLITE_OK)
    {
        cerr << "Error opening database: " << sqlite3_errmsg(db) << endl;
        close(client_socket);
        return;
    }
    char *errMsg;
    if (sqlite3_exec(db, "PRAGMA journal_mode=WAL;", NULL, 0, &errMsg) != SQLITE_OK)
    {
        cerr << "SQL error: " << errMsg << endl;
        sqlite3_free(errMsg);
    }
    else
    {
        std::cout << "WAL mode enabled." << endl;
    }
    std::cout << "Connected to client" << endl;

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
            std::cout << "Invalid format" << endl;
            send(client_socket, "N_format", strlen("N_format"), 0);
            continue;
        }

        string command = lower(received.substr(0, space_pos));
        string args = received.substr(space_pos + 1);
        if (command == "login")
        {
            size_t comma_pos = args.find(',');
            if (comma_pos == string::npos)
            {
                std::cout << "Invalid format" << endl;
                send(client_socket, "N_format", strlen("N_format"), 0);
                continue;
            }

            string username = args.substr(0, comma_pos);
            string password = args.substr(comma_pos + 1);
            std::cout << "Login requested" << endl;
            log_in(client_socket, username, password);
        }
        else if (command == "register")
        {
            size_t comma_pos = args.find(',');
            if (comma_pos == string::npos)
            {
                std::cout << "Invalid format" << endl;
                send(client_socket, "N_format", strlen("N_format"), 0);
                continue;
            }

            string username = args.substr(0, comma_pos);
            string password = args.substr(comma_pos + 1);

            std::cout << "Registration requested" << endl;
            register_user(client_socket, username, password);
        }
    }

    std::cout << "Connection closed" << endl;
    sqlite3_close(db);
    db = nullptr;
    close(client_socket);
}

void log_in(int client_socket, const string &username, const string &password) // Log in function
{
    if (userSocketMap.find(username) != userSocketMap.end())
    {
        std::cout << "User already logged in" << endl;
        send(client_socket, "N_login1", strlen("N_login1"), 0);
        return;
    }

    if (username == "admin" && password == "1")
    {
        send(client_socket, "Y_admin", strlen("Y_admin"), 0);
        admin_mode(client_socket);
    }
    else
    {

        sqlite3_stmt *stmt;
        string query = "SELECT username, password FROM Users WHERE username = ? AND password = ?";
        if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            cerr << "Error preparing query: " << sqlite3_errmsg(db) << endl;
            sqlite3_finalize(stmt);
            return;
        }

        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            User user;
            user.username = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            user.password = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            std::cout << "Login successful for user: " << user.username << endl;
            send(client_socket, "Y_login", strlen("Y_login"), 0);

            {
                std::lock_guard<std::mutex> lock(mapMutex);
                userSocketMap[username] = client_socket;
            } // The mutex is automatically released here
            sqlite3_finalize(stmt);
            functions(client_socket, user);
        }
        else
        {
            std::cout << "Login failed" << endl;
            send(client_socket, "N_login", strlen("N_login"), 0);
        }
        // sqlite3_finalize(stmt);
    }
}

void register_user(int client_socket, const string &username, const string &password)
{
    sqlite3_stmt *stmt;
    string query = "SELECT username FROM Users WHERE username = ?";
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        cerr << "Error preparing query: " << sqlite3_errmsg(db) << endl;
        sqlite3_finalize(stmt);
        return;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        std::cout << "Username already exists" << endl;
        send(client_socket, "N_register", strlen("N_register"), 0);
    }
    else
    {
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
        User newUser;
        newUser.username = username;
        newUser.password = password;
        std::cout << "Registration successful for user: " << newUser.username << endl;
        send(client_socket, "Y_register", strlen("Y_register"), 0);
        {
            std::lock_guard<std::mutex> lock(mapMutex);
            userSocketMap[username] = client_socket;
        }
        sqlite3_finalize(stmt);
        functions(client_socket, newUser);
    }
}

void admin_mode(int client_socket)
{
    sqlite3_stmt *stmt;
    char buffer[BUFFER_SIZE];
    int bytes_received;

    while (true)
    {
        bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0)
        {
            break;
        }

        buffer[bytes_received] = '\0';
        string received(buffer);

        if (lower(received) == "exit")
        {
            break;
        }
        if (lower(received) == "logout")
        {
            std::cout << "Logout requested" << endl;
            send(client_socket, "O_log", strlen("O_log"), 0);
            return;
        }
        vector<string> type1 = split(received, ' ');
        if (type1.size() < 2 || (lower(type1[0]) != "add_flight" && lower(type1[0]) != "del_flight" && lower(type1[0]) != "modify") && lower(type1[0]) != "y_noti")
        {
            std::cout << "Invalid format" << endl;
            send(client_socket, "N_ad", strlen("N_ad"), 0);
            continue;
        }
        if (lower(type1[0]) == "y_noti")
        {
            continue;
        }
        if (lower(type1[0]) == "add_flight")
        {
            string add_message = received.substr(11);
            vector<string> insert_params = split(add_message, ',');
            if (insert_params.size() == 10)
            {
                Flights newFlight;
                try
                {
                    newFlight.company = insert_params[0];
                    newFlight.flight_num = insert_params[1];
                    newFlight.num_A = stoi(insert_params[2]);
                    newFlight.num_B = stoi(insert_params[3]);
                    newFlight.price_A = stoi(insert_params[4]);
                    newFlight.price_B = stoi(insert_params[5]);
                    newFlight.departure_point = insert_params[6];
                    newFlight.destination_point = insert_params[7];
                    newFlight.departure_date = insert_params[8];
                    newFlight.return_date = insert_params[9];
                }
                catch (const std::invalid_argument &ia)
                {
                    std::cout << "Invalid argument: " << ia.what() << endl;
                    send(client_socket, "N_add", strlen("N_add"), 0);
                    continue;
                }
                string check_query = "SELECT flight_num FROM Flights WHERE flight_num = ?";
                if (sqlite3_prepare_v2(db, check_query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
                {
                    std::cout << "Error preparing query: " << sqlite3_errmsg(db) << endl;
                    send(client_socket, "N_add", strlen("N_add"), 0);
                    continue;
                }
                sqlite3_bind_text(stmt, 1, newFlight.flight_num.c_str(), -1, SQLITE_STATIC);
                if (sqlite3_step(stmt) == SQLITE_ROW)
                {
                    std::cout << "Flight number already exists" << endl;
                    send(client_socket, "N_add", strlen("N_add"), 0);
                    sqlite3_finalize(stmt);
                    continue;
                }
                sqlite3_finalize(stmt);

                string insert_query = "INSERT INTO Flights (company, flight_num, seat_class_A,seat_class_B,price_A,price_B, departure_point, destination_point, departure_date, return_date) VALUES (?, ?, ?, ?, ?, ?, ?,?,?,?)";
                if (sqlite3_prepare_v2(db, insert_query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
                {
                    cerr << "Error preparing query: " << sqlite3_errmsg(db) << endl;
                    send(client_socket, "N_add", strlen("N_add"), 0);
                    continue;
                }

                // Binding parameters to avoid SQL injection
                sqlite3_bind_text(stmt, 1, newFlight.company.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 2, newFlight.flight_num.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_int(stmt, 3, newFlight.num_A);
                sqlite3_bind_int(stmt, 4, newFlight.num_B);
                sqlite3_bind_int(stmt, 5, newFlight.price_A);
                sqlite3_bind_int(stmt, 6, newFlight.price_B);
                sqlite3_bind_text(stmt, 7, newFlight.departure_point.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 8, newFlight.destination_point.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 9, newFlight.departure_date.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 10, newFlight.return_date.c_str(), -1, SQLITE_STATIC);

                if (sqlite3_step(stmt) != SQLITE_DONE)
                {
                    cerr << "Error inserting data: " << sqlite3_errmsg(db) << endl;
                    send(client_socket, "N_add", strlen("N_add"), 0);
                }
                else
                {
                    send(client_socket, "Y_add", strlen("Y_add"), 0);
                }
                sqlite3_finalize(stmt);
            }
            else
            {
                send(client_socket, "N_add", strlen("N_add"), 0);
            }
        }
        else if (lower(type1[0]) == "del_flight")
        {
            string del_message = received.substr(11);
            vector<string> delete_params = split(del_message, ',');
            if (delete_params.size() == 1)
            {
                string flight_num = delete_params[0];
                const char *flight_check_sql = "SELECT COUNT(*) FROM Flights WHERE flight_num = ?;";
                if (sqlite3_prepare_v2(db, flight_check_sql, -1, &stmt, NULL) != SQLITE_OK)
                {
                    cerr << "Error preparing flight check statement" << endl;
                    return;
                }
                else
                {
                    sqlite3_bind_text(stmt, 1, flight_num.c_str(), -1, SQLITE_TRANSIENT);
                    if (sqlite3_step(stmt) == SQLITE_ROW)
                    {
                        if (sqlite3_column_int(stmt, 0) == 0)
                        {
                            std::cout << "Flight number does not exist" << endl;
                            send(client_socket, "N_del", strlen("N_del"), 0);
                            sqlite3_finalize(stmt);
                            return;
                        }
                    }
                    sqlite3_finalize(stmt);
                }
                // Check for tickets for this flight
                string ticket_check_query = "SELECT user_id FROM Tickets WHERE flight_num = ?";
                if (sqlite3_prepare_v2(db, ticket_check_query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
                {
                    cerr << "Error preparing query: " << sqlite3_errmsg(db) << endl;
                    send(client_socket, "N_del", strlen("N_del"), 0);
                    continue;
                }
                sqlite3_bind_text(stmt, 1, flight_num.c_str(), -1, SQLITE_STATIC);

                vector<int> affected_user_ids; // users who are in that flight
                while (sqlite3_step(stmt) == SQLITE_ROW)
                {
                    affected_user_ids.push_back(sqlite3_column_int(stmt, 0));
                }
                sqlite3_finalize(stmt);

                // Delete the flight
                string delete_query = "DELETE FROM Flights WHERE flight_num = ?";
                if (sqlite3_prepare_v2(db, delete_query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
                {
                    cerr << "Error preparing query: " << sqlite3_errmsg(db) << endl;
                    send(client_socket, "N_del", strlen("N_del"), 0);
                    continue;
                }
                sqlite3_bind_text(stmt, 1, flight_num.c_str(), -1, SQLITE_STATIC);

                if (sqlite3_step(stmt) != SQLITE_DONE)
                {
                    cerr << "Error deleting flight: " << sqlite3_errmsg(db) << endl;
                    send(client_socket, "N_del", strlen("N_del"), 0);
                }
                else
                {
                    std::cout << "Flight deleted successfully" << endl;
                    send(client_socket, "Y_del", strlen("Y_del"), 0);
                }
                sqlite3_finalize(stmt);

                // Notify affected users
                {
                    std::lock_guard<std::mutex> lock(mapMutex);
                    for (int user_id : affected_user_ids)
                    {
                        string username = get_username_from_id(user_id);
                        auto it = userSocketMap.find(username);
                        if (it != userSocketMap.end())
                        {
                            int user_socket = it->second;
                            string notification = "Y_noti_cancelled" + flight_num;
                            send(user_socket, notification.c_str(), notification.length(), 0);
                        }
                        else
                        {
                            cerr << "User " << username << " not connected for notification." << endl;
                        }
                    }
                }

                // Delete the tickets for this flight
                string del_ticket_query = "DELETE FROM Tickets WHERE flight_num = ?";
                if (sqlite3_prepare_v2(db, del_ticket_query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
                {
                    cerr << "Error preparing query: " << sqlite3_errmsg(db) << endl;
                    send(client_socket, "N_del", strlen("N_del"), 0);
                    continue;
                }
                sqlite3_bind_text(stmt, 1, flight_num.c_str(), -1, SQLITE_STATIC);

                if (sqlite3_step(stmt) != SQLITE_DONE)
                {
                    cerr << "Error deleting tickets: " << sqlite3_errmsg(db) << endl;
                    send(client_socket, "N_del", strlen("N_del"), 0);
                }
                else
                {
                    std::cout << "Tickets for flight " << flight_num << " deleted successfully" << endl;
                }
                sqlite3_finalize(stmt);
            }
            else
            {
                send(client_socket, "N_del", strlen("N_del"), 0);
            }
        }
    }
}

void functions(int client_socket, const User &user)
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
            {
                std::lock_guard<std::mutex> lock(mapMutex);
                userSocketMap.erase(user.username);
            }
            break;
        }
        if (received == "logout")
        {
            cerr << "Logout requested" << endl;
            send(client_socket, "O_log", strlen("O_log"), 0);
            {
                std::lock_guard<std::mutex> lock(mapMutex);
                userSocketMap.erase(user.username);
            }
            return;
        }
        if(lower(received)=="view"){
            received+= " ";
        }
        size_t space_pos = received.find(' ');
        if (space_pos == string::npos)
        {
            std::cout << "Invalid format" << endl;
            send(client_socket, "N_in", strlen("N_in"), 0);
            continue;
        }
        vector<string> type1 = split(received, ' ');
        if (lower(type1[0]) != "y_noti" && (lower(type1[0]).compare("search") != 0 && type1.size()<2) && lower(type1[0]).compare("book") != 0 && lower(type1[0]).compare("view") != 0 && lower(type1[0]).compare("print") != 0 && lower(type1[0]).compare("pay") != 0 && lower(type1[0]).compare("cancel") != 0 && lower(type1[0]).compare("change") != 0)
        {
            std::cout << "Invalid format" << endl;
            send(client_socket, "N_in", strlen("N_in"), 0);
            continue;
        }
        if (lower(type1[0]) == "y_noti")
        {
            continue;
        }
        if (lower(type1[0]) == "search")
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
                std::cout << error_message << endl;
                send(client_socket, "N_search", strlen("N_search"), 0);
            }
        }
        else if (lower(type1[0]) == "book")
        {
            vector<string> book_params = split(type1[1], ',');
            if (book_params.size() == 2)
            {
                string flight_num = book_params[0];
                string seat_class = book_params[1];

                book_flight(client_socket, flight_num, seat_class, user);
            }
            else
            {
                string error_message = "Invalid format for booking. Please provide necessary details.";
                std::cout << error_message << endl;
                send(client_socket, "N_book_miss", strlen("N_booK_miss"), 0);
            }
        }
        else if (type1[0] == "view")
        {
            cerr << "view";
            sqlite3_stmt *stmt;
            string query = "SELECT T.ticket_code, T.flight_num, T.seat_class, T.ticket_price, T.payment, F.company, F.departure_date, F.return_date, F.departure_point, F.destination_point "
                           "FROM Tickets T "
                           "JOIN Flights F ON T.flight_num = F.flight_num "
                           "JOIN Users U ON T.user_id = U.user_id "
                           "WHERE U.username = ?";

            if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
            {
                cerr << "Error preparing query: " << sqlite3_errmsg(db) << endl;
                send(client_socket, "N_view", strlen("N_view"), 0);
                return;
            }

            sqlite3_bind_text(stmt, 1, user.username.c_str(), -1, SQLITE_STATIC);

            string result_str = "Y_view/";
            bool found = false;

            while (sqlite3_step(stmt) == SQLITE_ROW)
            {
                found = true;
                Ticket ticket;
                Flights flight;
                ticket.ticket_code = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
                ticket.flight_num = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
                ticket.seat_class = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
                ticket.ticket_price = sqlite3_column_int(stmt, 3);
                ticket.payment = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
                flight.company = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));
                flight.departure_date = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 6));
                flight.return_date = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 7));
                flight.departure_point = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 8));
                flight.destination_point = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 9));

                stringstream ss;
                ss << ticket.ticket_price;
                string str = ss.str();
                string str_ticket_price = str.substr(0, 3) + "." + str.substr(3, 3);

                result_str += ticket.flight_num + ",";
                result_str += ticket.ticket_code + ",";
                result_str += flight.company + ",";
                result_str += flight.departure_point + ',';
                result_str += flight.destination_point + ',';
                result_str += flight.departure_date + ",";
                result_str += flight.return_date + ",";
                result_str += ticket.seat_class + ",";
                result_str += str_ticket_price + "VND" + ",";
                result_str += ticket.payment + ";";
            }

            sqlite3_finalize(stmt);

            if (!found)
            {
                send(client_socket, "N_view", strlen("N_view"), 0);
            }
            else
            {
                send(client_socket, result_str.c_str(), result_str.length(), 0);
            }
        }

        else if (lower(type1[0]) == "cancel")
        {
            if (type1.size() == 2)
            {
                string ticket_code = type1[1];
                cancel_flight(client_socket, ticket_code);
            }
            else
            {
                string error_message = "Missing element!";
                std::cout << error_message << endl;
                send(client_socket, "N_cancel_miss", strlen("N_cancel_miss"), 0);
            }
        }
        else if (lower(type1[1])=="print" && (type1[1]) == "all")
        {   
            print_all(client_socket, user);
        }
        else if( lower(type1[1])=="print" &&lower(type1[1])!="all"){
            print_ticket(client_socket, type1[1], user);
        }
        else if (lower(type1[0]) == "pay")
        {
            int ticket_price;
            vector<string> pay_params = split(type1[1], ',');
            sqlite3_stmt *stmt;
            string check = "SELECT ticket_price FROM Tickets WHERE ticket_code = ?";

            sqlite3_prepare_v2(db, check.c_str(), -1, &stmt, NULL);
            sqlite3_bind_text(stmt, 1, pay_params[0].c_str(), -1, SQLITE_TRANSIENT);

            if (sqlite3_step(stmt) == SQLITE_ROW)
            {
                ticket_price = sqlite3_column_int(stmt, 0);
            }

            sqlite3_finalize(stmt);

            string update_pay = "UPDATE Tickets SET payment = 'PAID' WHERE ticket_code = ?";

            if (sqlite3_prepare_v2(db, update_pay.c_str(), -1, &stmt, NULL) != SQLITE_OK)
            {
                cerr << "Error preparing update statement" << endl;
                send(client_socket, "N_pay", strlen("N_pay"), 0);
            }
            else
            {
                if (sqlite3_bind_text(stmt, 1, pay_params[0].c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK)
                {
                    cerr << "Error binding ticket_code to update statement" << endl;
                    send(client_socket, "N_pay", strlen("N_pay"), 0);
                }
                else
                {
                    if (sqlite3_step(stmt) != SQLITE_DONE)
                    {
                        cerr << "Error executing update statement" << endl;
                        send(client_socket, "N_pay", strlen("N_pay"), 0);
                    }
                }
                sqlite3_finalize(stmt);
                string Y_pay = "Y_pay/" + to_string(ticket_price) + pay_params[0];
                send(client_socket, Y_pay.c_str(), Y_pay.length(), 0);
            }
        }
        else if (lower(type1[0]) == "change")
        {   
            {
            vector<string> change_params = split(type1[1], ',');
            if (change_params.size() == 3)
            {
                change_flight(client_socket, change_params[0], change_params[1], change_params[2], user);
            }
            else
            {
                string error_message = "Invalid format for booking. Please provide necessary details.";
                std::cout << error_message << endl;
                send(client_socket, "N_change", strlen("N_change"), 0);
            }
        }

        }
    }
    close(client_socket);
}

void search_flight(int client_socket, const string &departure_point, const string &destination_point)
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
    bool found = false;

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        found = true;
        Flights flight;
        flight.company = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        flight.flight_num = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        flight.num_A = sqlite3_column_int(stmt, 2);
        flight.num_B = sqlite3_column_int(stmt, 3);
        flight.price_A = sqlite3_column_int(stmt, 4);
        flight.price_B = sqlite3_column_int(stmt, 5);
        flight.departure_point = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 6));
        flight.destination_point = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 7));
        flight.departure_date = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 8));
        flight.return_date = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 9));

        result_str += flight.company + ",";
        result_str += flight.flight_num + ",";
        result_str += to_string(flight.num_A) + ",";
        result_str += to_string(flight.num_B) + ",";
        result_str += to_string(flight.price_A) + " VND" + ",";
        result_str += to_string(flight.price_B) + " VND" + ",";
        result_str += flight.departure_point + ",";
        result_str += flight.destination_point + ",";
        result_str += flight.departure_date + ",";
        result_str += flight.return_date + ";";
    }

    if (!found)
    {
        send(client_socket, "N_found", strlen("N_found"), 0);
    }
    else
    {
        send(client_socket, result_str.c_str(), result_str.length(), 0);
    }

    sqlite3_finalize(stmt);
}

void book_flight(int client_socket, const string flight_num, const string seat_class, const User &user)
{
    sqlite3_stmt *stmt;
    int ticket_price = 0;
    string query_price = "SELECT ";
    query_price += (seat_class == "A" ? "price_A" : "price_B");
    query_price += " FROM Flights WHERE flight_num = ?";

    sqlite3_prepare_v2(db, query_price.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, flight_num.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        ticket_price = sqlite3_column_int(stmt, 0);
    }
    else
    {
        cerr << "Failed to retrieve ticket price." << endl;
    }

    string query_seat;
    int available_seats;

    if (seat_class == "A")
    {
        query_seat = "SELECT seat_class_A FROM Flights WHERE flight_num = ?";
    }
    else if (seat_class == "B")
    {
        query_seat = "SELECT seat_class_B FROM Flights WHERE flight_num = ?";
    }
    else
    {
        send(client_socket, "N_invalid_class", strlen("N_invalid_class"), 0);
        return;
    }

    if (sqlite3_prepare_v2(db, query_seat.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        cerr << "Error preparing query: " << sqlite3_errmsg(db) << endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, flight_num.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        available_seats = sqlite3_column_int(stmt, 0);
    }
    else
    {
        send(client_socket, "N_flight_not_found", strlen("N_flight_not_found"), 0);
        sqlite3_finalize(stmt);
        return;
    }
    sqlite3_finalize(stmt);
    std::cout << "Seat available: " << available_seats << endl;
    if (available_seats == 0)
    {
        string N_book_avail = "N_no_seats/" + seat_class;
        send(client_socket, N_book_avail.c_str(), N_book_avail.length(), 0);
        return;
    }
    string query = "SELECT flight_num FROM Flights WHERE flight_num = ?";
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        cerr << "Error preparing query: " << sqlite3_errmsg(db) << endl;
        send(client_socket, "N_book", strlen("N_book"), 0);
        return;
    }
    sqlite3_bind_text(stmt, 1, flight_num.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        sqlite3_finalize(stmt);

        int user_id = -1;
        string query1 = "SELECT user_id FROM Users WHERE username = ?";
        if (sqlite3_prepare_v2(db, query1.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt, 1, user.username.c_str(), -1, SQLITE_STATIC);

            if (sqlite3_step(stmt) == SQLITE_ROW)
            {
                user_id = sqlite3_column_int(stmt, 0);
            }
            else
            {
                cerr << "No user found" << endl;
                send(client_socket, "N_book", strlen("N_book"), 0);
                sqlite3_finalize(stmt);
                return;
            }
            sqlite3_finalize(stmt);
        }
        else
        {
            cerr << "Error preparing user query: " << sqlite3_errmsg(db) << endl;
            send(client_socket, "N_book", strlen("N_book"), 0);
            sqlite3_finalize(stmt);
            return;
        }

        string ticket_code = generate_ticket_code();

        string payment_status = "NOT_PAID";
        string query2 = "INSERT INTO Tickets (ticket_code, user_id, flight_num, seat_class, ticket_price,payment) VALUES (?, ?, ?, ?, ?,?)";
        if (sqlite3_prepare_v2(db, query2.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt, 1, ticket_code.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 2, user_id);
            sqlite3_bind_text(stmt, 3, flight_num.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 4, seat_class.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 5, ticket_price);
            sqlite3_bind_text(stmt, 6, payment_status.c_str(), -1, SQLITE_STATIC);

            if (sqlite3_step(stmt) == SQLITE_DONE)
            {
                std::cout << "Finished booking\n";
                string success_book = "Y_book/";
                success_book += ticket_code;
                success_book += to_string(ticket_price) + "VND";
                send(client_socket, success_book.c_str(), success_book.length(), 0);

                update_seat_count(db, flight_num, seat_class, -1);
                sqlite3_finalize(stmt);
            }
            else
            {
                cerr << "Error inserting ticket data: " << sqlite3_errmsg(db) << endl;
                send(client_socket, "N_book", strlen("N_book"), 0);
            }
        }
        else
        {
            cerr << "Error preparing insert query: " << sqlite3_errmsg(db) << endl;
            send(client_socket, "N_book", strlen("N_book"), 0);
        }
    }
    else
    {
        send(client_socket, "N_book", strlen("N_book"), 0);
    }
}
void cancel_flight(int client_socket, const string ticket_code)
{
    sqlite3_stmt *stmt;
    string flight_num, seat_class;

    string query = "SELECT flight_num, seat_class FROM Tickets WHERE ticket_code = ?";
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        cerr << "Error preparing query: " << sqlite3_errmsg(db) << endl;
        send(client_socket, "N_cancel_err", strlen("N_cancel_err"), 0);
        return;
    }

    sqlite3_bind_text(stmt, 1, ticket_code.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        flight_num = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        seat_class = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
    }
    else
    {
        cerr << "Ticket not found" << endl;
        send(client_socket, "N_cancel_notfound", strlen("N_cancel_notfound"), 0);
        sqlite3_finalize(stmt);
        return;
    }
    sqlite3_finalize(stmt);

    query = "DELETE FROM Tickets WHERE ticket_code = ?";
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        cerr << "Error preparing delete query: " << sqlite3_errmsg(db) << endl;
        send(client_socket, "N_cancel_err", strlen("N_cancel_err"), 0);
        return;
    }

    sqlite3_bind_text(stmt, 1, ticket_code.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        cerr << "Error deleting ticket: " << sqlite3_errmsg(db) << endl;
        send(client_socket, "N_cancel_err", strlen("N_cancel_err"), 0);
    }
    else
    {
        std::cout << "Ticket cancelled successfully" << endl;
        string cancel_success = "Y_cancel/" + ticket_code;
        send(client_socket, cancel_success.c_str(), cancel_success.length(), 0);

        update_seat_count(db, flight_num, seat_class, 1);
    }
    sqlite3_finalize(stmt);
}

void change_flight(int client_socket, const string old_ticket_code, const string flight_num_new, const string seat_class_new, const User &user)
{   
    string change_success = "Y_change/";
    sqlite3_stmt *stmt;
    string old_flight_num, old_seat_class;

    string can_query = "SELECT flight_num, seat_class FROM Tickets WHERE ticket_code = ?";
    if (sqlite3_prepare_v2(db, can_query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        cerr << "Error preparing query: " << sqlite3_errmsg(db) << endl;
        send(client_socket, "N_cancel_err", strlen("N_cancel_err"), 0);
        return;
    }

    sqlite3_bind_text(stmt, 1, old_ticket_code.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        old_flight_num = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        old_seat_class = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
    }
    else
    {
        cerr << "Ticket not found" << endl;
        send(client_socket, "N_found_change", strlen("N_found_change"), 0);
        sqlite3_finalize(stmt);
        return;
    }
    sqlite3_finalize(stmt);
    can_query = "DELETE FROM Tickets WHERE ticket_code = ?";
    if (sqlite3_prepare_v2(db, can_query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        cerr << "Error preparing delete query: " << sqlite3_errmsg(db) << endl;
        send(client_socket, "N_cancel_err", strlen("N_cancel_err"), 0);
        return;
    }

    sqlite3_bind_text(stmt, 1, old_ticket_code.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        cerr << "Error deleting ticket: " << sqlite3_errmsg(db) << endl;
        send(client_socket, "N_cancel_err", strlen("N_cancel_err"), 0);
        return;
    }

    std::cout << "Ticket cancelled successfully" << endl;
    change_success += old_ticket_code;

    update_seat_count(db, old_flight_num, old_seat_class, 1);
    sqlite3_finalize(stmt);

    int ticket_price = 0;
    string query_price = "SELECT ";
    query_price += (seat_class_new == "A" ? "price_A" : "price_B");
    query_price += " FROM Flights WHERE flight_num = ?";

    sqlite3_prepare_v2(db, query_price.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, flight_num_new.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        ticket_price = sqlite3_column_int(stmt, 0);
    }
    else
    {
        cerr << "Failed to retrieve ticket price." << endl;
    }

    string query_seat;
    int available_seats;

    if (seat_class_new == "A")
    {
        query_seat = "SELECT seat_class_A FROM Flights WHERE flight_num = ?";
    }
    else if (seat_class_new == "B")
    {
        query_seat = "SELECT seat_class_B FROM Flights WHERE flight_num = ?";
    }
    else
    {
        send(client_socket, "N_invalid_class", strlen("N_invalid_class"), 0);
        return;
    }

    if (sqlite3_prepare_v2(db, query_seat.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        cerr << "Error preparing query: " << sqlite3_errmsg(db) << endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, flight_num_new.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        available_seats = sqlite3_column_int(stmt, 0);
    }
    else
    {
        send(client_socket, "N_flight_not_found", strlen("N_flight_not_found"), 0);
        sqlite3_finalize(stmt);
        return;
    }
    sqlite3_finalize(stmt);
    std::cout << "Seat available: " << available_seats << endl;
    if (available_seats == 0)
    {
        string N_book_avail = "N_no_seats/" + seat_class_new;
        send(client_socket, N_book_avail.c_str(), N_book_avail.length(), 0);
        return;
    }

    string book_query = "SELECT flight_num FROM Flights WHERE flight_num = ?";
    if (sqlite3_prepare_v2(db, book_query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        cerr << "Error preparing query: " << sqlite3_errmsg(db) << endl;
        send(client_socket, "N_book", strlen("N_book"), 0);
        return;
    }
    sqlite3_bind_text(stmt, 1, flight_num_new.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        sqlite3_finalize(stmt);

        int user_id = -1;
        string query1 = "SELECT user_id FROM Users WHERE username = ?";
        if (sqlite3_prepare_v2(db, query1.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt, 1, user.username.c_str(), -1, SQLITE_STATIC);

            if (sqlite3_step(stmt) == SQLITE_ROW)
            {
                user_id = sqlite3_column_int(stmt, 0);
            }
            else
            {
                cerr << "No user found" << endl;
                send(client_socket, "N_book", strlen("N_book"), 0);
                sqlite3_finalize(stmt);
                return;
            }
            sqlite3_finalize(stmt);
        }
        else
        {
            cerr << "Error preparing user query: " << sqlite3_errmsg(db) << endl;
            send(client_socket, "N_book", strlen("N_book"), 0);
            sqlite3_finalize(stmt);
            return;
        }

        string new_ticket_code = generate_ticket_code();

        // Payment
        string payment_status = "NOT_PAID";
        string query2 = "INSERT INTO Tickets (ticket_code, user_id, flight_num, seat_class, ticket_price, payment) VALUES (?, ?, ?, ?, ?,?)";
        if (sqlite3_prepare_v2(db, query2.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt, 1, new_ticket_code.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 2, user_id);
            sqlite3_bind_text(stmt, 3, flight_num_new.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 4, seat_class_new.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 5, ticket_price);
            sqlite3_bind_text(stmt, 6, payment_status.c_str(), -1, SQLITE_STATIC);

            if (sqlite3_step(stmt) == SQLITE_DONE)
            {
                std::cout << "Finished booking\n";
                change_success += new_ticket_code;
                change_success += to_string(ticket_price) + "VND";
                send(client_socket, change_success.c_str(), change_success.length(), 0);
                update_seat_count(db, flight_num_new, seat_class_new, -1);
            }
            else
            {
                cerr << "Error inserting ticket data: " << sqlite3_errmsg(db) << endl;
                send(client_socket, "N_book", strlen("N_book"), 0);
            }
        }
        else
        {
            cerr << "Error preparing insert query: " << sqlite3_errmsg(db) << endl;
            send(client_socket, "N_book", strlen("N_book"), 0);
        }
        sqlite3_finalize(stmt);
    }
    else
    {
        send(client_socket, "N_book", strlen("N_book"), 0);
    }
}

void update_seat_count(sqlite3 *db, const string &flight_num, const string &seat_class, int adjustment)
{
    sqlite3_stmt *stmt;
    string sql;
    if (seat_class == "A")
    {
        sql = "UPDATE Flights SET seat_class_A = seat_class_A + ? WHERE flight_num = ?";
    }
    else if (seat_class == "B")
    {
        sql = "UPDATE Flights SET seat_class_B = seat_class_B + ? WHERE flight_num = ?";
    }
    else
    {
        cerr << "Invalid seat class" << endl;
        return;
    }

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        cerr << "SQL error: " << sqlite3_errmsg(db) << endl;
        return;
    }

    sqlite3_bind_int(stmt, 1, adjustment);
    sqlite3_bind_text(stmt, 2, flight_num.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        cerr << "SQL error in updating seat count: " << sqlite3_errmsg(db) << endl;
    }

    sqlite3_finalize(stmt);
}

std::string get_username_from_id(int user_id)
{
    sqlite3_stmt *stmt;
    std::string username;
    std::string query = "SELECT username FROM Users WHERE user_id = ?";

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, user_id);

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const unsigned char *username_raw = sqlite3_column_text(stmt, 0);
            username = reinterpret_cast<const char *>(username_raw);
        }
        else
        {
            std::cerr << "User not found for ID: " << user_id << std::endl;
        }

        sqlite3_finalize(stmt);
    }
    else
    {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    }

    return username;
}
void print_all(int client_socket, const User &user){
                sqlite3_stmt *stmt;
            string query = "SELECT T.ticket_code, T.flight_num, T.seat_class, T.ticket_price, F.company, F.departure_date, F.return_date, F.departure_point, F.destination_point "
                           "FROM Tickets T "
                           "JOIN Flights F ON T.flight_num = F.flight_num "
                           "JOIN Users U ON T.user_id = U.user_id "
                           "WHERE U.username = ?";

            if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
            {
                cerr << "Error preparing query: " << sqlite3_errmsg(db) << endl;
                send(client_socket, "N_view", strlen("N_view"), 0);
                return;
            }

            sqlite3_bind_text(stmt, 1, user.username.c_str(), -1, SQLITE_STATIC);

            string result_str = "Y_print/";
            bool found = false;

            while (sqlite3_step(stmt) == SQLITE_ROW)
            {
                found = true;
                Ticket ticket;
                Flights flight;
                ticket.ticket_code = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
                ticket.flight_num = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
                ticket.seat_class = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
                ticket.ticket_price = sqlite3_column_int(stmt, 3);
                flight.company = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
                flight.departure_date = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));
                flight.return_date = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 6));
                flight.departure_point = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 7));
                flight.destination_point = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 8));

                stringstream ss;
                ss << fixed << setprecision(2) << ticket.ticket_price;
                string formatted_price = ss.str();

                result_str += ticket.flight_num + ",";
                result_str += ticket.ticket_code + ",";
                result_str += flight.company + ",";
                result_str += flight.departure_point + ',';
                result_str += flight.destination_point + ',';
                result_str += flight.departure_date + ",";
                result_str += flight.return_date + ",";
                result_str += ticket.seat_class + ",";
                result_str += formatted_price + "VND" + ";";
            }

            sqlite3_finalize(stmt);

            if (!found)
            {
                send(client_socket, "N_view", strlen("N_view"), 0);
            }
            else
            {
                send(client_socket, result_str.c_str(), result_str.length(), 0);
            }
}

void print_ticket(int client_socket, const string ticket_code, const User &user){
 sqlite3_stmt *stmt;
            string query = "SELECT T.ticket_code, T.flight_num, T.seat_class, T.ticket_price, F.company, F.departure_date, F.return_date, F.departure_point, F.destination_point "
                           "FROM Tickets T "
                           "JOIN Flights F ON T.flight_num = F.flight_num "
                           "WHERE T.ticket_code = ?";

            if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
            {
                cerr << "Error preparing query: " << sqlite3_errmsg(db) << endl;
                send(client_socket, "N_print_cer", strlen("N_print_cer"), 0);
                return;
            }

            sqlite3_bind_text(stmt, 1, ticket_code.c_str(), -1, SQLITE_STATIC);

            string result_str = "Y_print_cer/";
            bool found = false;

            while (sqlite3_step(stmt) == SQLITE_ROW)
            {
                found = true;
                Ticket ticket;
                Flights flight;
                ticket.ticket_code = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
                ticket.flight_num = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
                ticket.seat_class = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
                ticket.ticket_price = sqlite3_column_int(stmt, 3);
                flight.company = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
                flight.departure_date = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));
                flight.return_date = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 6));
                flight.departure_point = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 7));
                flight.destination_point = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 8));

                stringstream ss;
                ss << fixed << setprecision(2) << ticket.ticket_price;
                string formatted_price = ss.str();

                result_str += ticket.flight_num + ",";
                result_str += ticket.ticket_code + ",";
                result_str += flight.company + ",";
                result_str += flight.departure_point + ',';
                result_str += flight.destination_point + ',';
                result_str += flight.departure_date + ",";
                result_str += flight.return_date + ",";
                result_str += ticket.seat_class + ",";
                result_str += formatted_price + "VND" + ";";
            }

            sqlite3_finalize(stmt);

            if (!found)
            {
                send(client_socket, "N_print_cer", strlen("N_print_cer"), 0);
            }
            else
            {
                send(client_socket, result_str.c_str(), result_str.length(), 0);
            }   
}