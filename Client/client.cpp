#include "client.h"

std::string tmp_noti = "Notification: ";

int main()
{
    const char *host = "127.0.0.1"; // Default for local test
    struct sockaddr_in server_addr;
    int client_socket;
    Role cur_role = Role::none;

    try
    {
        client_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (client_socket == -1)
        {
            throw runtime_error("Error creating client socket");
        }
        
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(SERV_PORT);
        if (inet_pton(AF_INET, host, &(server_addr.sin_addr)) <= 0)
        {
            cerr << "Invalid IP address" << endl;
            return 1;
        }

        if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        {
            throw runtime_error("Error connecting to the server");
        }
        
        char buffer[BUFFER_SIZE];

        while (true)
        {

            print_main_menu();
            string choice;
            getline(cin, choice);
            string lower_choice = trim(lower(choice));
            if (lower_choice == "exit")
            {
                send(client_socket, lower_choice.c_str(), choice.length(), 0);
                break;
            }
            else if (lower_choice == "login")
            {
                string username, password;
                cout << "Enter username: ";
                getline(cin, username);
                cout << "Enter password: ";
                getline(cin, password);
                string msg = "login " + username + "," + password;
                send(client_socket, msg.c_str(), msg.length(), 0);
            }
            else if (lower_choice == "register")
            {
                string username, password;
                cout << "Enter new username: ";
                getline(cin, username);
                cout << "Enter new password: ";
                getline(cin, password);
                string msg = "register " + username + "," + password;
                send(client_socket, msg.c_str(), msg.length(), 0);
            }
            else
            {
                std::cout << "Invalid choice!\n";
                continue;
            }
            memset(buffer, 0, BUFFER_SIZE);
            int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if (bytes_received <= 0)
            {
                break;
            }
            buffer[bytes_received] = '\0';

            string message(buffer);
            string response = string(buffer);

            if (response == "Y_login" || response == "Y_register")
            {
                cout << "You're currently online!" << endl;
                cur_role = Role::user;
                while (true)
                {   // std::lock_guard<std::mutex> lock(mapMutex);
                        std::cout << tmp_noti << std::endl;
                    
                    print_functions();
                    string choice1;
                    getline(cin, choice1);
                    string lower_choice1 = trim(lower(choice1));
                    if (lower_choice1 == " exit")
                    {
                        send(client_socket, lower_choice.c_str(), choice.length(), 0);
                        break;
                    }
                    else if (lower_choice1 == "search")
                    {
                        string company, destination_point, departure_point, departure_date, return_date;
                        cout << "Choose criteria to search:" << endl;
                        cout << "Enter company (or leave blank for any): ";
                        getline(cin, company);

                        cout << "Enter departure point (or leave blank for any): ";
                        getline(cin, departure_point);

                        cout << "Enter destination point (or leave blank for any): ";
                        getline(cin, destination_point);

                        cout << "Enter departure date (or leave blank for any, format YYYY-MM-DD): ";
                        getline(cin, departure_date);

                        cout << "Enter return date (or leave blank for any, format YYYY-MM-DD): ";
                        getline(cin, return_date);

                        string search_msg;
                        if (!departure_point.empty() && !destination_point.empty() && company.empty() && departure_date.empty() && return_date.empty())
                        {
                            search_msg += "search1 " + departure_point + "," + destination_point;
                        }
                        else if (!departure_point.empty() && !destination_point.empty() && !departure_date.empty() && company.empty() && return_date.empty())
                        {
                            search_msg += "search2 " + departure_point + "," + destination_point + "," + departure_date;
                        }
                        else if (!company.empty() && !departure_point.empty() && !destination_point.empty() && departure_date.empty() && return_date.empty())
                        {
                            search_msg += "search3 " + company + "," + departure_point + "," + destination_point;
                        }

                        else if (!departure_point.empty() && !destination_point.empty() && !departure_date.empty() && !return_date.empty() && company.empty())
                        {
                            search_msg += "search4 " + departure_point + "," + destination_point + "," + departure_date + "," + return_date;
                        }
                        else if (!company.empty() && !departure_point.empty() && !destination_point.empty() && !departure_date.empty() && !return_date.empty())
                        {
                            search_msg += "search5 " + company + "," + departure_point + "," + destination_point + "," + departure_date + "," + return_date;
                        }
                        send(client_socket, search_msg.c_str(), search_msg.length(), 0);
                        memset(buffer, 0, BUFFER_SIZE);
                        int bytes_received1 = recv(client_socket, buffer, BUFFER_SIZE, 0);
                        if (bytes_received1 <= 0)
                        {
                            break;
                        }
                        buffer[bytes_received1] = '\0';

                        string response1 = string(buffer);
                        if (response1.find("Y_found/") == 0)
                        {
                            string flight_data = response1.substr(8);
                            std::cout << "Flight data:" << endl;
                            size_t pos = 0;
                            while (true)
                            {
                                size_t next_pos = flight_data.find(';', pos);
                                if (next_pos == string::npos)
                                {
                                    break;
                                }
                                string flight_info = flight_data.substr(pos, next_pos - pos);
                                std::cout << flight_info << endl;
                                pos = next_pos + 1;
                            }
                        }
                        else if (response1 == "N_found")
                        {
                            std::cout << "Can't find the flight!\n";
                        }
                        else if (response1 == "N_search")
                        {
                            std::cout << "Input 2->6 elements for continuing searching" << endl;
                        }
                    }
                    else if (lower_choice1 == "book")
                    {
                        string flight_num, seat_class;
                        cout << "Enter your flight number: ";
                        getline(cin, flight_num);
                        cout << "Enter your desired seat class: ";
                        getline(cin, seat_class);
                        string book_msg = "book " + flight_num + "," + seat_class;
                        send(client_socket, book_msg.c_str(), book_msg.length(), 0);
                        memset(buffer, 0, BUFFER_SIZE);
                        int bytes_received1 = recv(client_socket, buffer, BUFFER_SIZE, 0);
                        if (bytes_received1 <= 0)
                        {
                            break;
                        }
                        buffer[bytes_received1] = '\0';

                        string response1 = string(buffer);
                        if (response1.find("Y_book/") == 0)
                        {
                            string ticket_code = response1.substr(7, 6);
                            string ticket_price1 = response1.substr(13, 3);
                            string ticket_price2 = response1.substr(16, 6);
                            std::cout << "You've booked successfully\n";
                            std::cout << "Your ticket code: " << ticket_code << endl;
                            std::cout << "You will have to pay " << ticket_price1 << "." << ticket_price2 << endl;
                        }
                        else if (response1 == "N_book")
                        {
                            std::cout << "Can't find your flight number" << endl;
                        }
                        else if (response1 == "N_book_miss")
                        {
                            std::cout << "Input flight number and seatclass for continue booking" << endl;
                        }
                        else if (response1.find("N_no_seats/") == 0)
                        {
                            std::cout << "No seat class " << response1.substr(11) << " available" << endl;
                        }
                        else if (response1 == "N_invalid_class")
                        {
                            std::cout << "Invalid seat class\n";
                        }
                        else if (response1 == "N_flight_not_found")
                        {
                            std::cout << "Can't find you flight number";
                        }
                    }
                    else if (lower_choice1 == "view")
                    {
                        send(client_socket, "view", strlen("view"), 0);
                        memset(buffer, 0, BUFFER_SIZE);
                        int bytes_received1 = recv(client_socket, buffer, BUFFER_SIZE, 0);
                        if (bytes_received1 <= 0)
                        {
                            break;
                        }
                        buffer[bytes_received1] = '\0';

                        string response1 = string(buffer);
                        // std::cout << "DEBUG: Received response: '" << buffer << "'" << std::endl;
                        if (response1.find("Y_view/") == 0)
                        {

                            string ticket_data = response1.substr(7);
                            std::cout << "Tickets information:" << endl;
                            size_t pos = 0;
                            while (true)
                            {
                                size_t next_pos = ticket_data.find(';', pos);
                                if (next_pos == string::npos)
                                {
                                    break;
                                }
                                string ticket_info = ticket_data.substr(pos, next_pos - pos);
                                std::cout << ticket_info << endl;
                                pos = next_pos + 1;
                            }
                        }
                    }
                    else if (lower_choice1 == "cancel")
                    {
                        string ticket_code;
                        cout << "Enter the ticket code for cancelling: ";
                        getline(cin, ticket_code);
                        string cancel_msg = "cancel " + ticket_code;
                        send(client_socket, cancel_msg.c_str(), cancel_msg.length(), 0);
                        memset(buffer, 0, BUFFER_SIZE);
                        int bytes_received1 = recv(client_socket, buffer, BUFFER_SIZE, 0);
                        if (bytes_received1 <= 0)
                        {
                            break;
                        }
                        buffer[bytes_received1] = '\0';

                        string response1 = string(buffer);

                        if (response1.find("Y_cancel/") == 0)
                        {

                            string ticket_code = response1.substr(9);
                            std::cout << "You've cancelled ticket: " << ticket_code << endl;
                        }
                        else if (response1 == "N_cancel_miss")
                        {
                            std::cout << "Input your ticket code for cancelling";
                        }
                        else if (response1 == "N_cancel_err" || response1 == "N_cancel_notfound")
                        {
                            std::cout << "Can't find your ticket" << endl;
                        }
                    }
                    else if (lower_choice1 == "print")
                    {
                        string choice2;
                        cout << "Do you want to print all or print a single ticket(input the ticket code): ";
                        getline(cin, choice2);
                        if (choice2 == "all")
                        {
                            string print_msg = "print all";
                            send(client_socket, print_msg.c_str(), print_msg.length(), 0);
                            memset(buffer, 0, BUFFER_SIZE);
                            int bytes_received1 = recv(client_socket, buffer, BUFFER_SIZE, 0);
                            if (bytes_received1 <= 0)
                            {
                                break;
                            }
                            buffer[bytes_received1] = '\0';

                            string response1 = string(buffer);
                            if (response1.find("Y_print/") == 0)
                            {

                                string ticket_data = response1.substr(8);
                                cout << "Saved to tickets.txt" << endl;
                                save_all_tickets_to_file(ticket_data);
                            }
                        }
                        else
                        {
                            string print_msg = "print " + choice2;
                            send(client_socket, print_msg.c_str(), print_msg.length(), 0);
                            memset(buffer, 0, BUFFER_SIZE);
                            int bytes_received1 = recv(client_socket, buffer, BUFFER_SIZE, 0);
                            if (bytes_received1 <= 0)
                            {
                                break;
                            }
                            buffer[bytes_received1] = '\0';

                            string response1 = string(buffer);
                            if (response1.find("Y_print_cer/") == 0)
                            {
                                string ticket_data = response1.substr(12);
                                std::cout << "Saved information about ticket " << response1.substr(19, 6) << " to " << response1.substr(19, 6) << ".txt\n";
                                save_tickets_to_file(ticket_data, response1.substr(19, 6));
                            }
                            else if (response1 == "N_print_cer")
                            {
                                std::cout << "Can't find your ticket to print\n";
                            }
                        }
                    }
                    else if (lower_choice1 == "pay")
                    {
                        string ticket_code;
                        cout << "Enter the ticket code you want to pay: ";
                        getline(cin, ticket_code);
                        string pay_msg = "pay " + ticket_code;
                        send(client_socket, pay_msg.c_str(), pay_msg.length(), 0);
                        memset(buffer, 0, BUFFER_SIZE);
                        int bytes_received1 = recv(client_socket, buffer, BUFFER_SIZE, 0);
                        if (bytes_received1 <= 0)
                        {
                            break;
                        }
                        buffer[bytes_received1] = '\0';

                        string response1 = string(buffer);
                        if (response1 == "N_pay")
                        {
                            std::cout << "Can't find your ticket code\n";
                        }
                        else if (response1.find("Y_pay/") == 0)
                        {
                            std::cout << "You've paid " << response1.substr(6, 3) << "." << response1.substr(9, 3) << " VND for ticket " << response1.substr(12, 6) << endl;
                        }
                    }
                    else if (lower_choice1 == "change")
                    {
                        string ticket_code, new_flight_num, new_seat_class;
                        cout << "Enter the ticket_code that you want to change: ";
                        getline(cin, ticket_code);
                        cout << "Enter the new flight number: ";
                        getline(cin, new_flight_num);
                        cout << "Enter the new seat class: ";
                        getline(cin, new_seat_class);
                        string change_msg = "change " + ticket_code + "," + new_flight_num + "," + new_seat_class;
                        send(client_socket, change_msg.c_str(), change_msg.length(), 0);
                        memset(buffer, 0, BUFFER_SIZE);
                        int bytes_received1 = recv(client_socket, buffer, BUFFER_SIZE, 0);
                        if (bytes_received1 <= 0)
                        {
                            break;
                        }
                        buffer[bytes_received1] = '\0';

                        string response1 = string(buffer);
                        if (response1.find("Y_change/") == 0)
                        {
                            std::cout << "Cancelled ticket " << response1.substr(9, 6) << endl;
                            std::cout << "Your new ticket code is " << response1.substr(15, 6) << endl;
                            std::cout << "You will have to pay: " << response1.substr(21, 3) << "." << response1.substr(24, 6) << endl;
                        }
                        else if (response1 == "N_change")
                        {
                            std::cout << "Please check the format\n";
                        }
                        else if (response1.find("N_no_seats/") == 0)
                        {
                            std::cout << "No seat class " << response1.substr(11) << " available" << endl;
                        }
                        else if (response1 == "N_invalid_class")
                        {
                            std::cout << "Invalid seat class\n";
                        }
                        else if (response1 == "N_flight_not_found")
                        {
                            std::cout << "Can't find you flight number";
                        }
                    }
                    else if (lower_choice1 == "logout")
                    {
                        send(client_socket, lower_choice1.c_str(), lower_choice1.length(), 0);
                        memset(buffer, 0, BUFFER_SIZE);
                        int bytes_received1 = recv(client_socket, buffer, BUFFER_SIZE, 0);
                        if (bytes_received1 <= 0)
                        {
                            break;
                        }
                        buffer[bytes_received1] = '\0';

                        string response1 = string(buffer);
                        if (response1 == "O_log")
                        {
                            cur_role = Role::none; // Resetting the role to none
                            std::cout << "You've logged out successfully!" << endl;
                            break;
                        }
                    }
                    else
                    {
                        std::cout << "Invalid choice\n";
                        continue;
                    }
                }
            }
            else if (response == "N_login")
            {
                cout << "Login failed. Please check your username and password." << endl;
            }
            else if (response == "N_register")
            {
                cout << "Registration failed. Username might already exist." << endl;
            }
            else if (response == "N_login1")
            {
                std::cout << "This account is currently online\n";
            }
            else if (response == "N_register")
            {
                std::cout << "Your username has already existed!" << endl;
            }else if(response.find("Y_notif_cancelled")==0){
                tmp_noti += "Your flight " + response.substr(17, 6) + " has been cancelled\n";
                send(client_socket, "y_noti", strlen("y_noti"), 0);
            }
            else if (response == "Y_admin")
            {
                string admin_choice;
                cur_role = Role::admin;
                std::cout << "You're the admin\n";
                while (true)
                {
                    print_admin_menu();
                    getline(cin, admin_choice);
                    string lower_choice2 = trim(lower(admin_choice));

                    if (lower_choice2 == "add flight")
                    {
                        string company, flight_num, seat_class_A, seat_class_B, seat_class_A_price, seat_class_B_price, departure_point, destination_point, departure_date, return_date;
                        std::cout << "Enter the company: ";
                        getline(cin, company);

                        std::cout << "Enter the flight number: ";
                        getline(cin, flight_num);

                        std::cout << "Enter the number of seat left for seat class A: ";
                        getline(cin, seat_class_A);

                        std::cout << "Enter the number of seat left for seat class B: ";
                        getline(cin, seat_class_B);

                        std::cout << "Enter the price for seat class A: ";
                        getline(cin, seat_class_A_price);

                        std::cout << "Enter the price for seat class B: ";
                        getline(cin, seat_class_B_price);

                        std::cout << "Enter the departure point: ";
                        getline(cin, departure_point);

                        std::cout << "Enter the destination point: ";
                        getline(cin, destination_point);

                        std::cout << "Enter the departure date: ";
                        getline(cin, departure_date);

                        std::cout << "Enter the return date: ";
                        getline(cin, return_date);
                        string add_msg = "add_flight " + company + "," + flight_num + "," + seat_class_A + "," + seat_class_B + "," + seat_class_A_price + "," + seat_class_B_price + "," + departure_point + "," + destination_point + "," + departure_date + "," + return_date;
                        send(client_socket, add_msg.c_str(), add_msg.length(), 0);
                        memset(buffer, 0, BUFFER_SIZE);
                        int bytes_received2 = recv(client_socket, buffer, BUFFER_SIZE, 0);
                        if (bytes_received2 <= 0)
                        {
                            break;
                        }
                        buffer[bytes_received2] = '\0';

                        string response2 = string(buffer);

                        if (response2 == "Y_add")
                        {
                            std::cout << "Successfully inserted\n";
                        }
                        else if (response2 == "N_add")
                        {
                            std::cout << "The flight number might already exist\n";
                        }
                    }
                    else if (lower_choice2 == "delete flight")
                    {
                        string del_flight_num;
                        std::cout << "Enter the flight number which you want to delete: ";
                        getline(cin, del_flight_num);
                        string ad_del_msg = "del_flight " + del_flight_num;
                        send(client_socket, ad_del_msg.c_str(), ad_del_msg.length(), 0);
                        memset(buffer, 0, BUFFER_SIZE);
                        int bytes_received2 = recv(client_socket, buffer, BUFFER_SIZE, 0);
                        if (bytes_received2 <= 0)
                        {
                            break;
                        }
                        buffer[bytes_received2] = '\0';

                        string response2 = string(buffer);

                        if (response2 == "N_del")
                        {
                            std::cout << "The flight_num does not exist\n";
                        }
                        else if (response2 == "Y_del")
                        {

                            std::cout << "Deleted \n";
                        }
                    }
                    else if (lower_choice2 == "modify flight")
                    {
                        string modify_flight_num, modify_departure_date, modify_return_date;
                        string modify_msg;

                        std::cout << "Enter the flight number you want to modify: ";
                        getline(cin, modify_flight_num);
                        std::cout << "Enter the new departure date (possibly blank): ";
                        getline(cin, modify_departure_date);
                        std::cout << "Enter the new return date (possibly blank): ";
                        getline(cin, modify_return_date);
                        if (!modify_departure_date.empty() && !modify_return_date.empty())
                        {
                            modify_msg += "modify3 " + modify_flight_num + "," + modify_departure_date + "," + modify_return_date;
                        }
                        if (!modify_departure_date.empty() && modify_return_date.empty())
                        {
                            modify_msg += "modify2 " + modify_flight_num + "," + modify_departure_date;
                        }
                        if (modify_departure_date.empty() && !modify_return_date.empty())
                        {
                            modify_msg += "modify1 " + modify_flight_num + "," + modify_return_date;
                        }
                        send(client_socket, modify_msg.c_str(), modify_msg.length(), 0);
                        int bytes_received2 = recv(client_socket, buffer, BUFFER_SIZE, 0);
                        if (bytes_received2 <= 0)
                        {
                            break;
                        }
                        buffer[bytes_received2] = '\0';

                        string response2 = string(buffer);

                        if (response2 == "N_modify")
                        {
                            std::cout << "Can't find the flight number\n";
                        }
                        else if (response2 == "Y_modify")
                        {
                            std::cout << "Modified successfully\n";
                        }
                    }

                    else if (lower_choice2 == "logout")
                    {
                        send(client_socket, lower_choice2.c_str(), lower_choice2.length(), 0);
                        memset(buffer, 0, BUFFER_SIZE);
                        int bytes_received2 = recv(client_socket, buffer, BUFFER_SIZE, 0);
                        if (bytes_received2 <= 0)
                        {
                            break;
                        }
                        buffer[bytes_received2] = '\0';

                        string response2 = string(buffer);
                        if (response2 == "O_log")
                        {
                            cur_role = Role::none; // Resetting the role to none
                            std::cout << "You've logged out successfully!" << endl;
                            break;
                        }
                    }
                    else
                    {
                        std::cout << "Invalid choice!\n";
                    }
                }
            }
        }

        close(client_socket);
        std::cout << "Closed the connection." << endl;
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}