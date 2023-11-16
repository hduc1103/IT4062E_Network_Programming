import socket

MAXLINE = 4096
SERV_PORT = 3000

def main():
    host = input("Enter the server's IP address: ")

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client_socket:
        try:
            client_socket.connect((host, SERV_PORT))
            print("Connected to the server. Type 'exit' to terminate.")
        except Exception as e:
            print(f"Error connecting to the server: {e}")
            return

        while True:
            choice = input("Do you want to login, register, or search flights? ").lower()

            if choice == 'exit':
                break

            client_socket.sendall(choice.encode())  

            if choice not in ["login", "register", "search"]:
                print("Invalid choice. Please enter 'login', 'register', or 'search'.")
                continue

            if choice == 'search':
                response = client_socket.recv(MAXLINE).decode('utf-8')
                print(response)
            else:
                username = input("Enter username: ")
                password = input("Enter password: ")

                client_socket.sendall(username.encode())
                client_socket.sendall(password.encode())

                response = client_socket.recv(MAXLINE).decode('utf-8')
                print(response)

        print("Closing the connection.")

if __name__ == "__main__":
    main()
