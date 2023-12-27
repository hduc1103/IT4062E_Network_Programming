#client.py

import socket

MAXLINE = 4096
SERV_PORT = 3000
host = "127.0.0.1" #default for local test
def main():
    menu ="1. Login\n2. Register\n3. Exit"
    menu1 = "1. Search Flights(currently you can only search with flight_num\n2. Book tickets\n3. Manage booked tickets\n4. Exit\n "

    #host = input("Enter the server's IP address: ")

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client_socket:
        try:
            client_socket.connect((host, SERV_PORT))
            print("Connected to the server. Type 'exit' to terminate.")
            show_menu = True
            
            while True: 
                if show_menu:
                    print(menu)
                
                choice = input("Your message: ")
                client_socket.send(choice.encode())
                
                if choice == "exit":
                    break
                
                message = client_socket.recv(MAXLINE).decode('utf-8')
                message1= message.split('/')
                if message == "N_format":
                    print("Invalid format. Please choose a valid option!")
                    show_menu = True  
                elif message == "Y_login":
                    print("You've logged in successfully!\n")
                    print(menu1)
                    show_menu = False  
                elif message == "N_login":
                    print("Please check your username and password!\n")
                    show_menu= True
                elif message == "Y_register":
                    print("You've register successfully!\n")
                    print(menu1)
                    show_menu = False 
                elif message== "N_register":
                    print("Your username has aleady existed!\n")     
                    show_menu= True             
                elif message=="N_in":
                    print("Invalid format!")
                    print(menu1)
                    show_menu = False
                elif message=="N_search":
                    print("Input 2->6 elements for continue searching")
                    print(menu1)
                    show_menu = False
                elif message=="N_find":
                    print("Cant find")
                    show_menu = False
                elif len(message1) >= 2:
                    print("Flight data:")
                    message2 = message1[1].split(';')
                    for i in range(len(message2) - 1):
                        print(message2[i])
                    show_menu= False
        except Exception as e:
            print(f"Error connecting to the server: {e}")
            return

        print("Closed the connection.")

if __name__ == "__main__":
    main()
