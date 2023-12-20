import sqlite3
import random
from datetime import datetime, timedelta
# Connect to the SQLite database (or create it if it doesn't exist)
conn = sqlite3.connect('flight_database.db')
cursor = conn.cursor()

cursor.execute('''
    CREATE TABLE IF NOT EXISTS flight (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
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

cursor.execute('''
    CREATE TABLE IF NOT EXISTS user (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        username TEXT,
        password TEXT
    )
''')
 
 #random data
def random_date():
    start_date = datetime(2023, 1, 1)
    end_date = datetime(2023, 12, 31)
    random_date = start_date + timedelta(days=random.randint(0, (end_date - start_date).days))
    return random_date.strftime('%Y-%m-%d')

def random_flight_number():
    return ''.join(random.choices('ABCDEFGHIJKLMNOPQRSTUVWXYZ', k=3)) + str(random.randint(100, 999))

flights_data = [
    ('Company A', random_flight_number(), random.randint(50, 300), 'CityX', 'CityY', random_date(), random_date(), 'Economy'),
    ('Company B', random_flight_number(), random.randint(50, 300), 'CityY', 'CityZ', random_date(), random_date(), 'Business'),
]

users_data = [
    ('user1', 'password1'),
    ('user2', 'password2'),
]

cursor.executemany('''
    INSERT INTO flight (company, flight_num, number_of_passenger, departure_points, destination_points, departure_date, return_date, seat_class)
    VALUES (?, ?, ?, ?, ?, ?, ?, ?)
''', flights_data)

cursor.executemany('''
    INSERT INTO user (username, password)
    VALUES (?, ?)
''', users_data)

# Commit changes and close the connection
conn.commit()
conn.close()

print("Random data inserted successfully!")
