# server.py

import socket
import json

from model.flight import *
from features.search import *

PORT = 3000
BUFFER_SIZE = 1024
path = "user_data.json"

def load_user_data():
    try:
        with open(path, 'r') as file:
            user_data = json.load(file)
    except FileNotFoundError:
        user_data = {}
    return user_data

def save_user_data(user_data):
    with open(path, 'w') as file:
        json.dump(user_data, file)


def handle_login(client_socket, user_data, username, password):
    if username in user_data and user_data[username] == password:
        print("Login successful")
        client_socket.send("Login successful".encode('utf-8'))
    else:
        print("Login failed")
        client_socket.send("Login failed".encode('utf-8'))

def handle_registration(client_socket, user_data, username, password):
    if username in user_data:
        client_socket.send("Username already exists. Please choose another.".encode('utf-8'))
    else:
        user_data[username] = password
        save_user_data(user_data)
        client_socket.send("Registration successful".encode('utf-8'))

def send_menu(client_socket):
    menu = "1. Search Flights\n2. Book tickets\n3Manage booked tickets\n4. Exit\nEnter your choice: "
    client_socket.send(menu.encode('utf-8'))
    
def you_are_in(client_socket):
    send_menu(client_socket)
    choice = client_socket.recv(BUFFER_SIZE).decode('uft-8')

def handle_flight_search(client_socket, current_list, search_criteria):
    res = search_flights(current_list, search_criteria)

    print("Search Results:")
    if res is not None:
        for flight_instance in res:
            result_str = f"{flight_instance.company}, {flight_instance.flight_num}, {flight_instance.seat_class}"
            client_socket.send(result_str.encode('utf-8'))
    else:
        client_socket.send("NotFound".encode('utf-8'))


def load_data(filename):
    current_list = []
    with open(filename, 'r') as file:
        for line in file:
            data = line.strip().split(',')
            flight.add_flight(current_list, *data)
    return current_list

def handle_client(client_socket, user_data):
    print(f"Connected to {client_socket.getpeername()}")

    while True:
        try:
            menu = "1. Login\n2. Register\n3. Exit\n"
            client_socket.send(menu.encode('utf-8'))
            received = client_socket.recv(BUFFER_SIZE).decode('utf-8').strip()
            print(f"Received login information: {received}")
            
            if received.lower() == "exit":
                break

            type = received.split(' ')

            if len(type) != 2 or type[0] not in ("login", "register"):
                print("Invalid login format. Please use: login <username>,<password> or register <username>,<password>")
                client_socket.send("Invalid login format. Please use: login <username>,<password>".encode('utf-8'))
                continue

            command, credentials = type[0], type[1]

            username, password = credentials.split(',')
            username = username.strip()
            password = password.strip()

            if command == "login":
                print("Login requested")
                handle_login(client_socket, user_data, username, password)
            elif command == "register":
                print("Registration requested")
                handle_registration(client_socket, user_data, username, password)
        except ConnectionResetError:
            break

    print(f"Connection closed by {client_socket.getpeername()}")
    client_socket.close()



def main():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind(('0.0.0.0', PORT))
    server_socket.listen(socket.SOMAXCONN)

    print(f"Server listening on port {PORT}...")

    user_data = load_user_data()
    while True:
        client_socket, client_addr = server_socket.accept()
        print(f"Accepted connection from {client_addr[0]}:{client_addr[1]}")

        try:
            handle_client(client_socket, user_data)
        except ConnectionResetError:
            pass

            server_socket.close()


if __name__ == "__main__":
    main()
