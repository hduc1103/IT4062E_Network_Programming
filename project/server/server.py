import socket
import json

PORT = 3000
BUFFER_SIZE = 1024
path= "user_data.json"


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

def handle_login(client_socket, user_data):
    username = client_socket.recv(BUFFER_SIZE).decode('utf-8')
    password = client_socket.recv(BUFFER_SIZE).decode('utf-8')

    if username in user_data and user_data[username] == password:
        client_socket.send("Login successful".encode('utf-8'))
    else:
        client_socket.send("Login failed".encode('utf-8'))

def handle_registration(client_socket, user_data):
    username = client_socket.recv(BUFFER_SIZE).decode('utf-8')
    password = client_socket.recv(BUFFER_SIZE).decode('utf-8')

    user_data[username] = password
    save_user_data(user_data)

    client_socket.send("Registration successful".encode('utf-8'))

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
            request_type = client_socket.recv(BUFFER_SIZE).decode('utf-8')

            if request_type == "login":
                handle_login(client_socket, user_data)
            elif request_type == "register":
                handle_registration(client_socket, user_data)
        except ConnectionResetError:
            pass

        print(f"Connection closed by {client_addr[0]}:{client_addr[1]}")
        client_socket.close()
        server_socket.close()

if __name__ == "__main__":
    main()
