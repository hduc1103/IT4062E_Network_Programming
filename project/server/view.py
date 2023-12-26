import sqlite3

conn = sqlite3.connect('flight_database.db')
cursor = conn.cursor()

cursor.execute('SELECT * FROM flights')
flight_data = cursor.fetchall()
print("Flight Table Data:")
for row in flight_data:
    print(row)

# Close the connection
conn.close()
