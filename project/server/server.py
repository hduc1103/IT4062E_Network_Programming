import socket
import json

from features import *
from model import *

from model.flight import *
from features.search import *

PORT = 3000
BUFFER_SIZE = 1024
path1 = "user_data.json"
path2 = "flight_data.txt"

def menu(client_socket, current_list):
    while True:
        client_socket.send("1. Search".encode('utf-8'))
        client_socket.send("2. Book".encode('utf-8'))
        client_socket.send("3. Exit".encode('utf-8'))

        choice = client_socket.recv(BUFFER_SIZE).decode('utf-8')

        if choice.lower() == "Search":
            search_criteria = {
                'company': 'VietNamAirline',
                'departure_points': 'DaNang',
                'seat_class': 'C'
            }
            handle_flight_search(client_socket, current_list, search_criteria)
        elif choice.lower() == "book":
            
            pass
        elif choice.lower() == "exit":
            break

def load_user_data():
    try:
        with open(path1, 'r') as file:
            user_data = json.load(file)
    except FileNotFoundError:
        user_data = {}
    return user_data


def save_user_data(user_data):
    with open(path1, 'w') as file:
        json.dump(user_data, file)


def handle_login(client_socket, user_data, current_list):
    username = client_socket.recv(BUFFER_SIZE).decode('utf-8')
    password = client_socket.recv(BUFFER_SIZE).decode('utf-8')

    if username in user_data and user_data[username] == password:
        client_socket.send("Login successful".encode('utf-8'))
        menu(client_socket, current_list)
    else:
        client_socket.send("Login failed".encode('utf-8'))


def handle_registration(client_socket, user_data, current_list):
    username = client_socket.recv(BUFFER_SIZE).decode('utf-8')
    password = client_socket.recv(BUFFER_SIZE).decode('utf-8')

    user_data[username] = password
    save_user_data(user_data)

    client_socket.send("Registration successful".encode('utf-8'))
    menu(client_socket, current_list)


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
    
def main():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind(('0.0.0.0', PORT))
    server_socket.listen(socket.SOMAXCONN)

    print(f"Server listening on port {PORT}...")

    user_data = load_user_data()

    current_list = load_data(path2)

    while True:
        client_socket, client_addr = server_socket.accept()
        print(f"Accepted connection from {client_addr[0]}:{client_addr[1]}")
        try:
            request_type = client_socket.recv(BUFFER_SIZE).decode('utf-8')
            if request_type == "login":
                handle_login(client_socket, user_data, current_list)
            elif request_type == "register":
                handle_registration(client_socket, user_data, current_list)
        except ConnectionResetError:
            pass

        print(f"Connection closed by {client_addr[0]}:{client_addr[1]}")
        client_socket.close()


if __name__ == "__main__":
    main()
