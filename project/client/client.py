import socket

MAXLINE = 4096
SERV_PORT = 3000

def main():
    host = input("Enter the server's IP address: ")

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client_socket:
        try:
            client_socket.connect((host, SERV_PORT))
            print("Connected to the server. Type 'exit' to terminate.")
            
            while True:
                message = client_socket.recv(MAXLINE).decode('utf-8')
                if message.lower() == 'exit':
                    break
                print(message)
                choice = input("Your message: ")
                client_socket.send(choice.encode())
        except Exception as e:
            print(f"Error connecting to the server: {e}")
            return

        print("Closing the connection.")

if __name__ == "__main__":
    main()
