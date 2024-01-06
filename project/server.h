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
#include <cstdlib> 
#include <ctime> 
#include <iomanip>

using namespace std;

#define PORT 3000
#define BUFFER_SIZE 1024

void log_in(int client_socket, const string& username, const string& password);
void register_user(int client_socket, const string& username, const string& password);
void search_flight(int client_socket, const string& departure_point, const string& destination_point) ;
void functions(int client_socket);
void connect_client(int client_socket);
void admin_mode(int client_socket);
void book_flight(int client_socket, const string flight_num, const string seat_class);
void cancel_flight(int client_socket, const string ticket_code);
string cur_user;

struct Flights {
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
    string seat_class;
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
string lower(const string& input) {
    string result = input;
    
    for (char &c : result) {
        c = tolower(c);
    }

    return result;
}
string generate_ticket_code() {
    srand(time(NULL));

    const string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const int alphabet_length = alphabet.length();

    string ticket_code;

    for (int i = 0; i < 3; ++i) {
        ticket_code += alphabet[rand() % alphabet_length];
    }
    for (int i = 0; i < 3; ++i) {
        ticket_code += to_string(rand() % 10);
    }
    return ticket_code;
}

