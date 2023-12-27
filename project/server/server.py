# server.py

import socket
import json
import sqlite3
import threading
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

def create_database():
    flight_data = [
        ("Bamboo Airways", "XLK753", 259, "Vinh", "CaMau", "15/11/2023", "15/04/2024", "A"),
        ("Jetstar Pacific", "UWP0DF", 305, "CanTho", "VungTau", "13/01/2024", "22/11/2024", "C"),
        ("Jetstar Pacific", "RZBEAF", 370, "HaNoi", "NgheAn", "15/03/2024", "03/06/2024", "A"),
        ("Jetstar Pacific", "BRLTJ9", 495, "DaNang", "DaLat", "18/01/2024", "14/04/2024", "B"),
        ("Vietjett Air", "XLK753", 259, "Vinh", "CaMau", "15/11/2023", "15/04/2024", "A"),

    ]

    conn = sqlite3.connect('flight_database.db')
    c = conn.cursor()
    
    #c.execute("DROP TABLE flights")
    c.execute('''
        CREATE TABLE IF NOT EXISTS flights (
            company TEXT,
            flight_num TEXT,
            number_of_passenger INTEGER,
            departure_points TEXT,
            destination_points TEXT,
            departure_date TEXT,
            return_date TEXT,
            seat_class TEXT
        )
    ''')

    # for flight in flight_data:
    #     c.execute('''
    #         INSERT INTO flights (company, flight_num, number_of_passenger, departure_points, destination_points, departure_date, return_date, seat_class)
    #         VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    #     ''', flight)
    
    
    # companies_to_delete = ("Company A", "Company B")
    # for company in companies_to_delete:
    #     c.execute('''
    #         DELETE FROM flights
    #         WHERE company = ?
    #     ''', (company,))

    conn.commit()
    conn.close()

def log_in(client_socket, user_data, username, password):
    if username in user_data and user_data[username] == password:
        print("Login successful")
        client_socket.send("Y_login".encode('utf-8'))
        functions(client_socket)
    else:
        print("Login failed")
        client_socket.send("N_login".encode('utf-8'))


def register(client_socket, user_data, username, password):
    if username in user_data:
        print("Username existed")
        client_socket.send("N_register".encode('utf-8'))
    else:
        user_data[username] = password
        save_user_data(user_data)
        print("Registration accepted")
        client_socket.send("Y_register".encode('utf-8'))
        functions(client_socket)


def functions(client_socket):
    while True:
        received1 = client_socket.recv(BUFFER_SIZE).decode('utf-8').strip()
        if received1.lower() == "exit":
            break
        type1 = received1.split(' ')
        if len(type1) < 2 or type1[0].lower() not in ("search", "book", "manage"):
            print("Invalid format")
            client_socket.send("N_in".encode('utf-8'))
            continue

        if type1[0].lower() == "search":
            search_params = type1[1].split(',')

            if len(type1) == 2:
                print(type1[1])
                search_criteria = {
                    'flight_num': type1[1]
                }
                search(client_socket, search_criteria)
            else:
                error_message = "Missing element!"
                print(error_message)
                client_socket.send("N_search".encode('utf-8'))

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
                client_socket.send("N_book".encode('utf-8'))

        elif type1[0].lower() == "manage":
            manage_params = type1[1].split(',')

def search(client_socket, search_criteria):
    try:
        conn = sqlite3.connect('flight_database.db')
        cursor = conn.cursor()
        sql = "SELECT * FROM flights WHERE flight_num = ?"
        params = [search_criteria.get('flight_num')]
        print(params)
        cursor.execute(sql, params)
        res = cursor.fetchall()
        print("Search Results:  ")
        result_str= ""
        if res:
            for tmp_flight in res:
                result_str += f"{tmp_flight[0]},{tmp_flight[1]},{tmp_flight[7]};"
        else:
            client_socket.send("N_found".encode('utf-8'))
            
        result_str = "Y_found/" + result_str
        print(result_str)
        client_socket.send(result_str.encode('utf-8'))

        conn.close()

    except sqlite3.Error as er:
        print(f"SQLite error: {er}")
        client_socket.close()
    except Exception as e:
        print(f"Error occurred during flight search: {str(e)}")
        client_socket.close()

def connect_client(client_socket, user_data):
    print(f"Connected to {client_socket.getpeername()}")

    while True:
        try:
            received = client_socket.recv(BUFFER_SIZE).decode('utf-8').strip()
            print(f"Received login information: {received}")

            if received.lower() == "exit":
                break

            type = received.split(' ')

            if len(type) != 2 or type[0].lower() not in ("login", "register"):
                print("Invalid format")
                client_socket.send("N_format".encode('utf-8'))
                continue

            username, password = type[1].split(',')
            username = username.strip()
            password = password.strip()

            if type[0].lower() == "login":
                print("Login requested")
                log_in(client_socket, user_data, username, password)
            elif type[0].lower() == "register":
                print("Registration requested")
                register(client_socket, user_data, username, password)
        except ConnectionResetError:
            break

    print(f"Connection closed by {client_socket.getpeername()}")
    client_socket.close()


def main():
    create_database()
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind(('0.0.0.0', PORT))
    server_socket.listen(socket.SOMAXCONN)

    print(f"Server listening on port {PORT}...")

    user_data = load_user_data()
    while True:
        client_socket, client_addr = server_socket.accept()
        print(f"Accepted connection from {client_addr[0]}:{client_addr[1]}")

        try:
            connect_client(client_socket, user_data)
        except ConnectionResetError:
            pass

        server_socket.close()


if __name__ == "__main__":
    main()
