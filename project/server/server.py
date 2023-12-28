# server.py

import socket
import sqlite3
from _thread import *
import threading
PORT = 3000
BUFFER_SIZE = 1024

def log_in(client_socket, username, password):
    conn = sqlite3.connect('flight_database.db')
    c = conn.cursor()
    
    query1 = "SELECT username, password FROM Users WHERE username = ? AND password = ?"
    c.execute(query1, (username, password))
    
    state = c.fetchone()  
    
    if state:
        print("Login successful")
        client_socket.send("Y_login".encode('utf-8'))
        functions(client_socket)
    else:
        print("Login failed")
        client_socket.send("N_login".encode('utf-8'))

    conn.close()

def register(client_socket, username, password):
    conn = sqlite3.connect('flight_database.db')
    c = conn.cursor()
    
    query1 = "SELECT username FROM Users WHERE username = ?"
    c.execute(query1, (username,))
    
    state = c.fetchone()
    
    if state:
        print("Username existed")
        client_socket.send("N_register".encode('utf-8'))
    else:
        print("Registration accepted")
        client_socket.send("Y_register".encode('utf-8'))
        query2 = "INSERT INTO Users (username, password) VALUES (?, ?)"
        c.execute(query2, (username, password))
        conn.commit() 
        conn.close()
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
        sql = "SELECT * FROM Flights WHERE flight_num = ?"
        params = [search_criteria.get('flight_num')]
        print(params)
        cursor.execute(sql, params)
        res = cursor.fetchall()
        print("Search Results:  ")
        result_str = ""
        if res:
            for tmp_flight in res:
                result_str += f"{tmp_flight[0]},{tmp_flight[1]},{tmp_flight[2]},{tmp_flight[3]},{tmp_flight[4]},{tmp_flight[5]};"
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


def connect_client(client_socket):
    print(f"Connected to {client_socket.getpeername()}")

    while True:
        try:
            received = client_socket.recv(BUFFER_SIZE).decode('utf-8').strip()
            print(f"Received information: {received}")

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
                log_in(client_socket, username, password)
            elif type[0].lower() == "register":
                print("Registration requested")
                register(client_socket, username, password)
        except ConnectionResetError:
            break

    print(f"Connection closed by {client_socket.getpeername()}")
    client_socket.close()

def main():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind(('0.0.0.0', PORT))
    server_socket.listen(socket.SOMAXCONN)
    print(f"Server listening on port {PORT}...")
    try:
        while True:
            client_socket, client_addr = server_socket.accept()
            print(f"Accepted connection from {client_addr[0]}:{client_addr[1]}")

            try:
                client_thread = threading.Thread(
                    target=connect_client, args=(client_socket,))
                client_thread.start()
            except ConnectionResetError:
                pass
    except KeyboardInterrupt:
        print("Closing server due to keyboard interrupt...")
    finally:
        server_socket.close()

if __name__ == "__main__":
    main()