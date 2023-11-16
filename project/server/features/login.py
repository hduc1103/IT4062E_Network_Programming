from model import user

def login(username, password):
    user_instance = user()

    if username in user_instance.users and user_instance.users[username] == password:
        print("You're in")
    else:
        print("Failed. Please check your username and password.")
