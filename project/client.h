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

void save_tickets_to_file(const string &ticket_data)
{
    ofstream file("tickets.txt");

    if (!file.is_open())
    {
        cerr << "Failed to open file for writing." << endl;
        return;
    }

    size_t pos = 0;
    while (true)
    {
        size_t next_pos = ticket_data.find(';', pos);
        if (next_pos == string::npos)
        {
            break;
        }
        string ticket_info = ticket_data.substr(pos, next_pos - pos);

        size_t start = 0, end;
        file << "---------------------" << endl;
        const char *titles[] = {"Flight Number: ", "Ticket Code: ", "Departure Point: ", "Destination Point: ", "Departure Date: ", "Return Date: ", "Seat Class: ", "Ticket Price: "};
        int field_index = 0;
        while ((end = ticket_info.find(',', start)) != string::npos)
        {
            string field = ticket_info.substr(start, end - start);
            file << titles[field_index++] << field << endl; // Writes each field with a title
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