import sqlite3

conn = sqlite3.connect('server/flight_database.db')
c = conn.cursor()

c.execute("SELECT * FROM Flights")
flights_info = c.fetchall()
print("Flights Table:")
for row in flights_info:
    print(row)

c.execute("SELECT * FROM Users")
users_info = c.fetchall()
print("\nUsers Table:")
for row in users_info:
    print(row)

c.execute("SELECT * FROM Tickets")
tickets_info = c.fetchall()
print("\nTickets Table:")
for row in tickets_info:
    print(row)

conn.commit()
conn.close()