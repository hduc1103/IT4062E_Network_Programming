#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>

using namespace std;

#define MAXLINE 4096
#define SERV_PORT 3000
#define BUFFER_SIZE 1024

void save_tickets_to_file(const string& ticket_data) {
    ofstream file("tickets.txt");

    if (!file.is_open()) {
        cerr << "Failed to open file for writing." << endl;
        return;
    }

    size_t pos = 0;
    while (true) {
        size_t next_pos = ticket_data.find(';', pos);
        if (next_pos == string::npos) {
            break;
        }
        string ticket_info = ticket_data.substr(pos, next_pos - pos);
        
        size_t start = 0, end;
        file << "---------------------" << endl;
        const char* titles[] = {"Flight Number: ", "Ticket Code: ", "Departure Point: ", "Destination Point: ", "Departure Date: ", "Return Date: ", "Seat Class: ", "Ticket Price: "};
        int field_index = 0;
        while ((end = ticket_info.find(',', start)) != string::npos) {
            string field = ticket_info.substr(start, end - start);
            file << titles[field_index++] << field << endl;  // Writes each field with a title
            start = end + 1;
        }
        file << "---------------------" << endl;

        pos = next_pos + 1;
    }

    file.close();
    cout << "Ticket information saved to tickets.txt" << endl;
}

string lower(const string &input)
{
    string result = input;

    for (char &c : result)
    {
        c = tolower(c);
    }

    return result;
}
enum class Role
{
    none,
    admin,
    user
};
void print_functions()
{
    std::cout << "1. Search Flights(search <departure_point>,<destination_point>)\n2. Book tickets\n3. View tickets detail\n4. Cancel tickets\n5. Change tickets\n6. Print tickets\n7. Log out\n8. Exit" << endl;
    std::cout << "Your message: ";
}
void print_admin_menu()
{
    std::cout << "1. Add flight\n2. Delete flight\n3. Modify flight\n4. Logout\n5. Exit" << endl;
    std::cout << "Your message: ";
}
void print_main_menu()
{
    std::cout << "1. Login\n2. Register\n3. Exit\nYour message: ";
}
int main()
{
    const char *host = "127.0.0.1"; // Default for local test

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
                std::cout << "Invalid format. Please choose a valid option!" << endl;
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
            else if (message == "N_login")
            {
                std::cout << "Please check your username and password!" << endl;
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
            }
            else if (message == "N_in")
            {
                std::cout << "Invalid format!" << endl;
                print_functions();
            }
            else if (message == "N_search")
            {
                std::cout << "Input 2->6 elements for continuing searching" << endl;
                print_functions();
            }
            else if (message == "N_found")
            {
                std::cout << "Can't find" << endl;
                print_functions();
            }
            else if (message.find("Y_found/") == 0)
            {
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
                string ticket_code = message.substr(7);
                std::cout << "You've booked successfully\n";
                std::cout << "Your ticket code: " << ticket_code << endl;
                print_functions();
            }
            else if (message == "N_book")
            {
                std::cout << "Can't find your flight number" << endl;
                print_functions();
            }
            else if (message == "N_book_miss")
            {
                std::cout << "Input flight number and seatclass for continue booking" << endl;
                print_functions();
            }
            else if (message.find("Y_view/") == 0)
            {
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
                string ticket_data = message.substr(8);
                cout << "Saved to tickets.txt" << endl;
                save_tickets_to_file(ticket_data);
                print_functions();
            }
            else if (message == "N_cancel_miss")
            {
                std::cout << "Input your ticket code for cancelling";
                print_functions();
            }
            else if (message == "N_cancel_err" || message == "N_cancel_notfound")
            {
                std::cout << "Can't find your ticket" << endl;
                print_functions();
            }
            else if (message.find("Y_cancel/") == 0)
            {
                string ticket_code = message.substr(9);
                std::cout << "You've cancelled ticket: " << ticket_code << endl;
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
