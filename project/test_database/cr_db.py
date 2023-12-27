#create database for testing and training with sqlite

import sqlite3

flight_data = [
    ("Bamboo Airways", "XLK753", 259, "Vinh", "CaMau", "15/11/2023", "15/04/2024", "A"),
    ("Jetstar Pacific", "UWP0DF", 305, "CanTho", "VungTau", "13/01/2024", "22/11/2024", "C"),
    ("Jetstar Pacific", "RZBEAF", 370, "HaNoi", "NgheAn", "15/03/2024", "03/06/2024", "A"),
    ("Jetstar Pacific", "BRLTJ9", 495, "DaNang", "DaLat", "18/01/2024", "14/04/2024", "B"),
    ("Company A", "BRLTJ9", 495, "DaNang", "DaLat", "18/01/2024", "14/04/2024", "B"),
    ("Company B", "BRLTJ9", 495, "DaNang", "DaLat", "18/01/2024", "14/04/2024", "B")

]

conn = sqlite3.connect('test_flight_database.db')
c = conn.cursor()
c.execute('''
    CREATE TABLE IF NOT EXISTS flights (
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

for flight in flight_data:
    c.execute('''
        INSERT INTO flights (company, flight_num, number_of_passenger, departure_points, destination_points, departure_date, return_date, seat_class)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    ''', flight)
companies_to_delete = ("Company A", "Company B")

for company in companies_to_delete:
    c.execute('''
        DELETE FROM flights
        WHERE company = ?
    ''', (company,))

conn.commit()
conn.close()

print("Flight data inserted successfully!")
