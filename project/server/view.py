import sqlite3

conn = sqlite3.connect('flight_database.db')
cursor = conn.cursor()

cursor.execute('SELECT * FROM flight')
flight_data = cursor.fetchall()
print("Flight Table Data:")
for row in flight_data:
    print(row)

cursor.execute('SELECT * FROM user')
user_data = cursor.fetchall()
print("\nUser Table Data:")
for row in user_data:
    print(row)

# Close the connection
conn.close()
