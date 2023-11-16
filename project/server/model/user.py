class user:
    def __init__(self):
        self.users = {}

    def signup(self, username, password):
        if username in self.users:
            print("Username already exists!")
        else:
            self.users[username] = password
            print("Finished registration")



#user_db = UserDatabase()

#user_db.add_user("john_doe", "password123")
#user_db.authenticate_user("john_doe", "password123")  # Should print "Authentication successful."

#user_db.add_user("jane_smith", "pass456")
#user_db.authenticate_user("jane_smith", "wrong_password")  # Should print "Authentication failed."
