#include "client.h"

int main()
{
    //   char host[20]; // Buffer to store the server IP address
    // cout << "Enter server IP address: ";
    // cin.getline(host, sizeof(host));
    const char *host = "127.0.0.1"; // Default for local test
    string tmp_noti = "Notification: ";

    struct sockaddr_in server_addr;
    int client_socket;
    Role cur_role = Role::none;
    print_main_menu();
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
            string choice;
            getline(cin, choice);

            if (lower(choice) == "exit")
            {
                send(client_socket, choice.c_str(), choice.length(), 0);

                break;
            }

            send(client_socket, choice.c_str(), choice.length(), 0);

            int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if (bytes_received <= 0)
            {
                break;
            }
            buffer[bytes_received] = '\0';

            string message(buffer);

            if (message == "N_format")
            {
                if (tmp_noti.length() > 15)
                {
                    std::cout << tmp_noti;
                    tmp_noti = "Notification: ";
                }
                std::cout << "Invalid format. Please choose a valid option!" << endl;
                print_main_menu();
            }
            else if (message == "Y_admin")
            {
                cur_role = Role::admin;
                std::cout << "You're the admin\n";
                print_admin_menu();
            }
            else if (message == "O_log")
            {
                cur_role = Role::none; // Resetting the role to none
                std::cout << "You've logged out successfully!" << endl;
                print_main_menu();
            }
            else if (message == "N_ad")
            {
                std::cout << "Please input add_flight, del_flight or modify\n";
                print_admin_menu();
            }
            else if (message == "Y_add")
            {
                std::cout << "Successfully inserted\n";
                print_admin_menu();
            }
            else if (message == "N_add")
            {
                std::cout << "Not inserted\n";
                print_admin_menu();
            }
            else if (message == "N_del")
            {
                std::cout << "The flight_num does not exist\n";
                print_admin_menu();
            }
            else if (message == "Y_del")
            {
                if (cur_role == Role::admin)
                {
                    std::cout << "Deleted \n";
                    print_admin_menu();
                }
                else if (cur_role == Role::user)
                {
                    std::cout << "Deleted kdnf ";
                }
            }
            else if (message == "Y_login")
            {
                cur_role = Role::user;
                std::cout << "You've logged in successfully!" << endl;
                print_functions();
            }
            else if (message == "N_login1")
            {
                std::cout << "This account is currently online\n";
                print_main_menu();
            }
            else if (message == "N_login")
            {
                std::cout << "Please check your username and password!" << endl;
                print_main_menu();
            }
            else if (message == "Y_register")
            {
                cur_role = Role::user;
                std::cout << "You've registered successfully!" << endl;
                print_functions();
            }
            else if (message == "N_register")
            {
                std::cout << "Your username has already existed!" << endl;
                print_main_menu();
            }
            else if (message == "N_in")
            {
                std::cout << "Invalid format!" << endl;
                print_functions();
            }
            else if (message == "N_search")
            {
                if (tmp_noti.length() > 15)
                {
                    std::cout << tmp_noti;
                    tmp_noti = "Notification: ";
                }
                std::cout << "Input 2->6 elements for continuing searching" << endl;
                print_functions();
            }
            else if (message == "N_found")
            {
                if (tmp_noti.length() > 15)
                {
                    std::cout << tmp_noti;
                    tmp_noti = "Notification: ";
                }
                std::cout << "Can't find" << endl;
                print_functions();
            }
            else if (message.find("Y_found/") == 0)
            {
                if (tmp_noti.length() > 15)
                {
                    std::cout << tmp_noti;
                    tmp_noti = "Notification: ";
                }
                string flight_data = message.substr(8);
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
                print_functions();
            }
            else if (message.find("Y_book/") == 0)
            {
                if (tmp_noti.length() > 15)
                {
                    std::cout << tmp_noti;
                    tmp_noti = "Notification: ";
                }
                string ticket_code = message.substr(7, 6);
                string ticket_price1 = message.substr(13, 3);
                string ticket_price2 = message.substr(16, 6);
                std::cout << "You've booked successfully\n";
                std::cout << "Your ticket code: " << ticket_code << endl;
                std::cout << "You will have to pay " << ticket_price1 << "." << ticket_price2 << endl;
                print_functions();
            }
            else if (message == "N_book")
            {
                std::cout << tmp_noti;
                if (tmp_noti.length() > 15)
                {
                    std::cout << tmp_noti;
                    tmp_noti = "Notification: ";
                }
                std::cout << "Can't find your flight number" << endl;
                print_functions();
            }
            else if (message == "N_flight_not_found")
            {
                if (tmp_noti.length() > 15)
                {
                    std::cout << tmp_noti;
                    tmp_noti = "Notification: ";
                }
                std::cout << "Can't find your flight number" << endl;
                print_functions();
            }
            else if (message == "N_book_miss")
            {
                if (tmp_noti.length() > 15)
                {
                    std::cout << tmp_noti;
                    tmp_noti = "Notification: ";
                }
                std::cout << "Input flight number and seatclass for continue booking" << endl;
                print_functions();
            }
            else if (message.find("Y_view/") == 0)
            {
                if (tmp_noti.length() > 15)
                {
                    std::cout << tmp_noti;
                    tmp_noti = "Notification: ";
                }
                string ticket_data = message.substr(7);
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
                print_functions();
            }
            else if (message.find("Y_print/") == 0)
            {
                if (tmp_noti.length() > 15)
                {
                    std::cout << tmp_noti;
                    tmp_noti = "Notification: ";
                }
                string ticket_data = message.substr(8);
                cout << "Saved to tickets.txt" << endl;
                save_all_tickets_to_file(ticket_data);
                print_functions();
            }
            else if (message.find("Y_print_cer/") == 0)
            {
                if (tmp_noti.length() > 15)
                {
                    std::cout << tmp_noti;
                    tmp_noti = "Notification: ";
                }
                string ticket_data = message.substr(8);
                std::cout << "Saved information about ticket " << message.substr(19, 6) << " to " << message.substr(19, 6) << ".txt\n";
                save_tickets_to_file(ticket_data, message.substr(19, 6));
                print_functions();
            }
            else if (message == "N_print_cer")
            {
                std::cout << "Can't find your ticket to print\n";
                print_functions();
            }

            else if (message == "N_cancel_miss")
            {
                if (tmp_noti.length() > 15)
                {
                    std::cout << tmp_noti;
                    tmp_noti = "Notification: ";
                }
                std::cout << "Input your ticket code for cancelling";
                print_functions();
            }
            else if (message == "N_cancel_err" || message == "N_cancel_notfound")
            {
                if (tmp_noti.length() > 15)
                {
                    std::cout << tmp_noti;
                    tmp_noti = "Notification: ";
                }
                std::cout << "Can't find your ticket" << endl;
                print_functions();
            }
            else if (message.find("Y_cancel/") == 0)
            {
                if (tmp_noti.length() > 15)
                {
                    std::cout << tmp_noti;
                    tmp_noti = "Notification: ";
                }
                string ticket_code = message.substr(9);
                std::cout << "You've cancelled ticket: " << ticket_code << endl;
                print_functions();
            }
            else if (message == "N_invalid_class")
            {
                if (tmp_noti.length() > 15)
                {
                    std::cout << tmp_noti;
                    tmp_noti = "Notification: ";
                }
                std::cout << "Invalid seat class\n";
                print_functions();
            }
            else if (message.find("N_no_seats/") == 0)
            {
                if (tmp_noti.length() > 15)
                {
                    std::cout << tmp_noti;
                    tmp_noti = "Notification: ";
                }
                string message_no_seats = message.substr(11);
                std::cout << "No seat class " << message_no_seats << " available" << endl;
                print_functions();
            }else if(message=="N_change"){
                if (tmp_noti.length() > 15)
                {
                    std::cout << tmp_noti;
                    tmp_noti = "Notification: ";
                }
                std::cout<<"Please check the format\n";
                print_functions();
            }
            else if (message.find("Y_change/") == 0)
            {
                if (tmp_noti.length() > 15)
                {
                    std::cout << tmp_noti;
                    tmp_noti = "Notification: ";
                }
                std::cout << "Cancelled ticket " << message.substr(9, 6) << endl;
                std::cout << "Your new ticket code is " << message.substr(15, 6) << endl;
                std::cout << "You will have to pay: " << message.substr(21, 3) << "." << message.substr(24, 6) << endl;
                print_functions();
            }
            else if (message == "N_for_change")
            {
              if (tmp_noti.length() > 15)
                {
                    std::cout << tmp_noti;
                    tmp_noti = "Notification: ";
                }
                std::cout << "Please recheck the format!\n";
                print_functions();
            }
            else if (message.find("Y_noti_cancelled") == 0)
            {
                string flight_num = message.substr(16, 6);
                tmp_noti += "Your flight " + flight_num + " has been cancelled\n";
                send(client_socket, "Y_noti", strlen("Y_noti"), 0);
            }
            else if (message == "N_pay")
            {
                if (tmp_noti.length() > 15)
                {
                    std::cout << tmp_noti;
                    tmp_noti = "Notification: ";
                }
                std::cout << "Can't find your ticket code\n";
                print_functions();
            }
            else if (message.find("Y_pay/") == 0)
            {
                if (tmp_noti.length() > 15)
                {
                    std::cout << tmp_noti;
                    tmp_noti = "Notification: ";
                }
                std::cout << "You've paid " << message.substr(6, 3) << "." << message.substr(9, 3) << " VND for ticket " << message.substr(12, 6) << endl;
                print_functions();
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