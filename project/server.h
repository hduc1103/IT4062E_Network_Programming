#include <iostream>
#include <cstring>
#include <string>
#include <thread>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sqlite3.h>
#include <cstring>
#include <vector>
#include <sstream>

using namespace std;

#define PORT 3000
#define BUFFER_SIZE 1024

void log_in(int client_socket, const string& username, const string& password);
void register_user(int client_socket, const string& username, const string& password);
void search_flight(int client_socket, const string& departure_point, const string& destination_point) ;
void functions(int client_socket);
void connect_client(int client_socket);

struct Flight {
    string flight_num;
    int number_of_passenger;
    string departure_point;
    string destination_point;
    string departure_date;
    string return_date;
};

struct User {
    int user_id;
    string username;
    string password;
};

struct Ticket {
    string ticket_code;
    int user_id;
    string flight_num;
    double ticket_price;
};

vector<string> split(const string& input, char delimiter) {
    vector<string> result;
    stringstream ss(input);
    string item;

    while (getline(ss, item, delimiter)) {
        result.push_back(item);
    }

    return result;
}