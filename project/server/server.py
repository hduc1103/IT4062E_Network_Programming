# server.py

import socket
import json

from model.flight import *
from features.search import *

PORT = 3000
BUFFER_SIZE = 1024
path = "user_data.json"
flight_path = "flight_data.txt"


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


def load_data(filename):
    tmp_flight_data = []
    with open(filename, 'r') as file:
        for line in file:
            data = line.strip().split(',')
            flight.add_flight(tmp_flight_data, *data)
    return tmp_flight_data


def handle_login(client_socket, user_data, username, password, tmp_flight_data):
    if username in user_data and user_data[username] == password:
        print("Login successful")
        client_socket.send(
            "Login successful, press enter for continuing\n".encode('utf-8'))
        you_are_in(client_socket, tmp_flight_data)
    else:
        print("Login failed")
        client_socket.send("Login failed".encode('utf-8'))


def handle_registration(client_socket, user_data, username, password, tmp_flight_data):
    if username in user_data:
        client_socket.send(
            "Username already exists. Please choose another.".encode('utf-8'))
    else:
        user_data[username] = password
        save_user_data(user_data)
        client_socket.send("Registration successful".encode('utf-8'))
        you_are_in(client_socket, tmp_flight_data)


def you_are_in(client_socket, tmp_flight_data):
    while True:
        menu = "1. Search Flights\n2. Book tickets\n3. Manage booked tickets\n4. Exit\n "
        client_socket.send(menu.encode('utf-8'))
        print(menu)

        received1 = client_socket.recv(BUFFER_SIZE).decode('utf-8').strip()
        if received1.lower() == "exit":
            break

        type1 = received1.split(' ')
        if len(type1) < 2 or type1[0].lower() not in ("login", "register"):
            error_message = "Invalid format. Please try again!"
            print(error_message)
            client_socket.send(error_message.encode('utf-8'))
            continue

        if type1[0].lower() == "search":
            search_params = type1[1].split(',')
            if len(search_params) >= 2:
                departure_place = search_params[0]
                departure_date = search_params[1]

                # Prepare criteria based on user input
                search_criteria = {
                    "departure_place": departure_place,
                    "departure_date": departure_date
                }

                if len(search_params) == 3:
                    if len(search_params[2]) == 3:
                        return_date = search_params[2]
                        search_criteria["return_date"] = return_date
                    else:
                        airline = search_params[2]
                        search_criteria["airline"] = airline

                elif len(search_params) >= 4:
                    pass

                handle_flight_search(
                    client_socket, tmp_flight_data, search_criteria)
            else:
                error_message = "Please provide at least departure place and date for search!"
                print(error_message)
                client_socket.send(error_message.encode('utf-8'))

        elif type1[0].lower() == "book":
            book_params = type1[1].split(',')
            if len(book_params) >= 3:
                flight_number = book_params[0]
                seat_number = book_params[1]

                confirmation_msg = "Booking successful!"
                client_socket.send(confirmation_msg.encode('utf-8'))
            else:
                error_message = "Invalid format for booking. Please provide necessary details."
                print(error_message)
                client_socket.send(error_message.encode('utf-8'))

        elif type1[0].lower() == "manage":
            manage_params = type1[1].split(',')


def handle_flight_search(client_socket, tmp_flight_data, search_criteria):
    try:
        res = search_flights(tmp_flight_data, search_criteria)

        print("Search Results:")
        if res:
            for flight_instance in res:
                result_str = f"{flight_instance.company}, {flight_instance.flight_num}, {flight_instance.seat_class}"
                client_socket.send(result_str.encode('utf-8'))
        else:
            client_socket.send("NotFound".encode('utf-8'))

    except Exception as e:
        error_message = f"Error occurred during flight search: {str(e)}"
        print(error_message)
        client_socket.send(error_message.encode('utf-8'))

    finally:
        client_socket.close()


def handle_client(client_socket, user_data, tmp_flight_data):
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

            if len(type) != 2 or type[0].lower() not in ("login", "register"):
                print(
                    "Invalid login format. Please use: login <username>,<password> or register <username>,<password>")
                client_socket.send(
                    "Invalid login format. Please use: login <username>,<password>".encode('utf-8'))
                continue

            username, password = type[1].split(',')
            username = username.strip()
            password = password.strip()

            if type[0].lower() == "login":
                print("Login requested")
                handle_login(client_socket, user_data, username,
                             password, tmp_flight_data)
            elif type[0].lower() == "register":
                print("Registration requested")
                handle_registration(client_socket, user_data,
                                    username, password, tmp_flight_data)
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
    tmp_flight_data = load_data(flight_path)
    while True:
        client_socket, client_addr = server_socket.accept()
        print(f"Accepted connection from {client_addr[0]}:{client_addr[1]}")

        try:
            handle_client(client_socket, user_data, tmp_flight_data)
        except ConnectionResetError:
            pass

        server_socket.close()


if __name__ == "__main__":
    main()
