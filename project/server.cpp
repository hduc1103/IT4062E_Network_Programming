#include "server.h"

sqlite3 *db;

void log_in(int client_socket, const string &username, const string &password) // Log in function
{
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
            return;
        }

        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            User user;
            user.username = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            user.password = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            cur_user = username;
            cout << "Login successful for user: " << user.username << endl;
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
        cout << "Username already exists" << endl;
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
        else
        {
            User newUser;
            newUser.username = username;
            newUser.password = password;
            cur_user = username;
            cout << "Registration successful for user: " << newUser.username << endl;
            send(client_socket, "Y_register", strlen("Y_register"), 0);
            functions(client_socket);
        }

        sqlite3_finalize(stmt);
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
        if (received == "logout")
        {
            cout << "Logout requested" << endl;
            send(client_socket, "O_log", strlen("O_log"), 0);
            return;
        }
        vector<string> type1 = split(received, ' ');
        if (type1.size() < 2 || (lower(type1[0]) != "add_flight" && lower(type1[0]) != "del_flight" && lower(type1[0]) != "modify"))
        {
            cout << "Invalid format" << endl;
            send(client_socket, "N_ad", strlen("N_ad"), 0);
            continue;
        }

        if (lower(type1[0]) == "add_flight")
        {
            vector<string> insert_params = split(type1[1], ',');
            if (insert_params.size() == 6)
            {
                Flights newFlight;
                try
                {
                    newFlight.flight_num = insert_params[0];
                    newFlight.num_A = stoi(insert_params[1]);
                    newFlight.num_B = stoi(insert_params[2]);
                    newFlight.price_A = stoi(insert_params[3]);
                    newFlight.price_B = stoi(insert_params[4]);
                    newFlight.departure_point = insert_params[5];
                    newFlight.destination_point = insert_params[6];
                    newFlight.departure_date = insert_params[7];
                    newFlight.return_date = insert_params[8];
                }
                catch (const std::invalid_argument &ia)
                {
                    cerr << "Invalid argument: " << ia.what() << endl;
                    send(client_socket, "N_add", strlen("N_add"), 0);
                    continue;
                }
                string check_query = "SELECT flight_num FROM Flights WHERE flight_num = ?";
                if (sqlite3_prepare_v2(db, check_query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
                {
                    cerr << "Error preparing query: " << sqlite3_errmsg(db) << endl;
                    send(client_socket, "N_add", strlen("N_add"), 0);
                    continue;
                }
                sqlite3_bind_text(stmt, 1, newFlight.flight_num.c_str(), -1, SQLITE_STATIC);
                if (sqlite3_step(stmt) == SQLITE_ROW)
                {
                    cout << "Flight number already exists" << endl;
                    send(client_socket, "N_add", strlen("N_add"), 0);
                    sqlite3_finalize(stmt);
                    continue;
                }
                sqlite3_finalize(stmt);

                string insert_query = "INSERT INTO Flights (flight_num, number_of_passenger, departure_point, destination_point, departure_date, return_date) VALUES (?, ?, ?, ?, ?, ?)";
                if (sqlite3_prepare_v2(db, insert_query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
                {
                    cerr << "Error preparing query: " << sqlite3_errmsg(db) << endl;
                    send(client_socket, "N_add", strlen("N_add"), 0);
                    continue;
                }

                // Binding parameters to avoid SQL injection
                sqlite3_bind_text(stmt, 1, newFlight.flight_num.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_int(stmt, 2, newFlight.num_A);
                sqlite3_bind_int(stmt, 3, newFlight.num_B);
                sqlite3_bind_int(stmt, 4, newFlight.price_A);
                sqlite3_bind_int(stmt, 5, newFlight.price_B);
                sqlite3_bind_text(stmt, 6, newFlight.departure_point.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 7, newFlight.destination_point.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 8, newFlight.departure_date.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 9, newFlight.return_date.c_str(), -1, SQLITE_STATIC);

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
            vector<string> delete_params = split(type1[1], ',');
            if (delete_params.size() == 1)
            {
                string flight_num = delete_params[0];

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

                //  notify users (not finished)
                // for(auto user:affected_user_ids){
                //     send(client_socket, "")
                // }
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
                    cout << "Flight deleted successfully" << endl;
                    send(client_socket, "Y_del", strlen("Y_del"), 0);
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
        if (received == "logout")
        {
            cout << "Logout requested" << endl;
            send(client_socket, "O_log", strlen("O_log"), 0);
            return;
        }
        vector<string> type1 = split(received, ' ');

        if (((lower(type1[0]).compare("search") != 0 && type1.size() < 2) && (lower(type1[0]).compare("book") != 0 && type1.size() < 2) && lower(type1[0]).compare("view") != 0) && lower(type1[0]).compare("print")!=0)
        {
            cout << "Invalid format" << endl;
            send(client_socket, "N_in", strlen("N_in"), 0);
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
                cout << error_message << endl;
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

                book_flight(client_socket, flight_num, seat_class);
            }
            else
            {
                string error_message = "Invalid format for booking. Please provide necessary details.";
                cout << error_message << endl;
                send(client_socket, "N_book", strlen("N_booK_miss"), 0);
            }
        }
        else if (type1[0] == "view")
{
    sqlite3_stmt *stmt;
    string query = "SELECT T.ticket_code, T.flight_num, T.seat_class, T.ticket_price, F.departure_date, F.return_date, F.departure_point, F.destination_point "
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

    sqlite3_bind_text(stmt, 1, cur_user.c_str(), -1, SQLITE_STATIC);

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
        ticket.ticket_price = sqlite3_column_double(stmt, 3);
        flight.departure_date = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        flight.return_date = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));
        flight.departure_point = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 6));
        flight.destination_point = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 7));

        stringstream ss;
        ss << fixed << setprecision(2) << ticket.ticket_price;
        string formatted_price = ss.str();
                    
        result_str += ticket.flight_num + ",";
        result_str += ticket.ticket_code + ",";
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

        else if (lower(type1[0]) == "cancel")
        {   
            vector <string> cancel_params = split(type1[1],',');
            if (cancel_params.size()==1){
                string ticket_code = cancel_params[0];
                cancel_flight(client_socket, ticket_code);
            }else{
                string error_message = "Missing element!";
                cout << error_message << endl;
                send(client_socket, "N_cancel_miss", strlen("N_cancel_miss"), 0);
            }
        }
        else if (lower(type1[0])== "print"){
                      sqlite3_stmt *stmt;
    string query = "SELECT T.ticket_code, T.flight_num, T.seat_class, T.ticket_price, F.departure_date, F.return_date, F.departure_point, F.destination_point "
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

    sqlite3_bind_text(stmt, 1, cur_user.c_str(), -1, SQLITE_STATIC);

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
        ticket.ticket_price = sqlite3_column_double(stmt, 3);
        flight.departure_date = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        flight.return_date = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));
        flight.departure_point = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 6));
        flight.destination_point = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 7));

        stringstream ss;
        ss << fixed << setprecision(2) << ticket.ticket_price;
        string formatted_price = ss.str();
                    
        result_str += ticket.flight_num + ",";
        result_str += ticket.ticket_code + ",";
        result_str += flight.departure_point + ',';
        result_str += flight.destination_point + ',';
        result_str += flight.departure_date + ",";
        result_str += flight.return_date + ",";
        result_str += ticket.seat_class + ",";
        result_str += formatted_price + "VND"+";";
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
        flight.flight_num = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        flight.num_A = sqlite3_column_int(stmt, 1);
        flight.num_B = sqlite3_column_int(stmt, 2);
        flight.price_A = sqlite3_column_int(stmt, 3);
        flight.price_B = sqlite3_column_int(stmt, 4);
        flight.departure_point = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));
        flight.destination_point = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 6));
        flight.departure_date = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 7));
        flight.return_date = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 8));

        result_str += flight.flight_num + ",";
        result_str += to_string(flight.num_A) + ",";
        result_str += to_string(flight.num_B) + ",";
        result_str += to_string(flight.price_A) + " VND" + ",";
        result_str += to_string(flight.price_B) + " VND"+ ",";
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

void book_flight(int client_socket, const string flight_num, const string seat_class)
{
    double ticket_price = (seat_class == "A") ? 300.00 : 150.00;

    sqlite3_stmt *stmt;
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
        sqlite3_finalize(stmt); // Finalize the first statement before reusing it.

        int user_id = -1; // Initialize user_id with a default value
        string query1 = "SELECT user_id FROM Users WHERE username = ?";
        if (sqlite3_prepare_v2(db, query1.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt, 1, cur_user.c_str(), -1, SQLITE_STATIC);

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

        string query2 = "INSERT INTO Tickets (ticket_code, user_id, flight_num, seat_class, ticket_price) VALUES (?, ?, ?, ?, ?)";
        if (sqlite3_prepare_v2(db, query2.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt, 1, ticket_code.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 2, user_id);
            sqlite3_bind_text(stmt, 3, flight_num.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 4, seat_class.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_double(stmt, 5, ticket_price);

            if (sqlite3_step(stmt) == SQLITE_DONE)
            {
                cout << "Finished booking\n";
                string success_book = "Y_book/";
                success_book += ticket_code;
                send(client_socket, success_book.c_str(), success_book.length(), 0);
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
void cancel_flight(int client_socket, const string ticket_code)
{
    sqlite3_stmt *stmt;
    string query = "SELECT ticket_code FROM Tickets WHERE ticket_code = ?";
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        cerr << "Error preparing query: " << sqlite3_errmsg(db) << endl;
        send(client_socket, "N_cancel_err", strlen("N_cancel_err"), 0);
        return;
    }

    sqlite3_bind_text(stmt, 1, ticket_code.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_ROW)
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
        cout << "Ticket cancelled successfully" << endl;
        string cancel_success = "Y_cancel/" + ticket_code;
        send(client_socket, cancel_success.c_str(), cancel_success.length(), 0);
    }
    sqlite3_finalize(stmt);
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

        string command = lower(received.substr(0, space_pos));
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
        cerr << "Error opening database: " << sqlite3_errmsg(db) << endl;
        return 1;
    }

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        cerr << "Error creating server socket" << endl;
        sqlite3_close(db);
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        cerr << "Error binding server socket" << endl;
        close(server_socket);
        sqlite3_close(db);
        return 1;
    }

    if (listen(server_socket, SOMAXCONN) == -1)
    {
        cerr << "Error listening on server socket" << endl;
        close(server_socket);
        sqlite3_close(db);
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
