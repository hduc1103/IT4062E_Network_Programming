#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

#define MAXLINE 4096
#define SERV_PORT 3000
#define BUFFER_SIZE 1024

int main()
{
    const char *host = "127.0.0.1"; // Default for local test

    struct sockaddr_in server_addr;
    int client_socket;

    cout << "1. Login\n2. Register\n3. Exit" << endl;

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
        bool show_menu = true;

        while (true)
        {
            if (show_menu)
            {
                cout << "Your message: ";
            }

            string choice;
            getline(cin, choice);

            if (choice == "exit")
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
                cout << "Invalid format. Please choose a valid option!" << endl;
                show_menu = true;
            }
            else if (message == "Y_login")
            {
                cout << "You've logged in successfully!" << endl;
                cout << "1. Search Flights(search <departure_point>,<destination_point>)\n2. Book tickets\n3. Manage booked tickets\n4. Exit" << endl;
                show_menu = false;
            }
            else if (message == "N_login")
            {
                cout << "Please check your username and password!" << endl;
                show_menu = true;
            }
            else if (message == "Y_register")
            {
                cout << "You've registered successfully!" << endl;
                cout << "1. Search Flights(search <departure_point>,<destination_point>)\n2. Book tickets\n3. Manage booked tickets\n4. Exit" << endl;
                show_menu = false;
            }
            else if (message == "N_register")
            {
                cout << "Your username has already existed!" << endl;
                show_menu = true;
            }
            else if (message == "N_in")
            {
                cout << "Invalid format!" << endl;
                cout << "1. Search Flights(search <departure_point>,<destination_point>)\n2. Book tickets\n3. Manage booked tickets\n4. Exit" << endl;
                show_menu = false;
            }
            else if (message == "N_search")
            {
                cout << "Input 2->6 elements for continuing searching" << endl;
                cout << "1. Search Flights(search <departure_point>,<destination_point>)\n2. Book tickets\n3. Manage booked tickets\n4. Exit" << endl;
                show_menu = false;
            }
            else if (message == "N_found")
            {
                cout << "Can't find" << endl;
                show_menu = false;
            }
            else if (message.find("Y_found/") == 0)
            {
                string flight_data = message.substr(8);
                cout << "Flight data:" << endl;
                size_t pos = 0;
                while (true)
                {
                    size_t next_pos = flight_data.find(';', pos);
                    if (next_pos == string::npos)
                    {
                        break;
                    }
                    string flight_info = flight_data.substr(pos, next_pos - pos);
                    cout << flight_info << endl;
                    pos = next_pos + 1;
                }
                show_menu = false;
            }
        }

        close(client_socket);
        cout << "Closed the connection." << endl;
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
