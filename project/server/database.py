import sqlite3
  
conn = sqlite3.connect('server/flight_database.db')
c = conn.cursor()

c.execute('''CREATE TABLE IF NOT EXISTS Flights (
    flight_num VARCHAR(20) PRIMARY KEY,
    number_of_passenger INT,
    departure_point VARCHAR(50),
    destination_point VARCHAR(50),
    departure_date DATE,
    return_date DATE
)
''')
flights_data = [
        ('ABC123', 150, 'CaMau', 'Vinh', '2023-01-15', '2023-01-20'),
        ('DEF456', 200, 'HaNoi', 'HoChiMinh', '2023-02-10', '2023-02-15'),
        ('GHI789', 100, 'NgheAn', 'CaoBang', '2023-03-05', '2023-03-10'),
        ('JKL012', 180, 'QuyNhon', 'HaiPhong', '2023-04-20', '2023-04-25'),
        ('MNO345', 120, 'ThanhHoa', 'KhanhHoa', '2023-05-12', '2023-05-18')
    ]
c.execute('''CREATE TABLE IF NOT EXISTS Users (
    user_id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT,
    password TEXT
)
''')

c.execute('''CREATE TABLE IF NOT EXISTS Tickets (
    ticket_code TEXT,
    user_id INTEGER,
    flight_num VARCHAR(20),
    ticket_price INTERGER,
    FOREIGN KEY (user_id) REFERENCES Users(user_id),
    FOREIGN KEY (flight_num) REFERENCES Flights(flight_num)
)
''')

c.executemany('''
        INSERT INTO Flights (flight_num, number_of_passenger, departure_point, destination_point, departure_date, return_date)
        VALUES (?, ?, ?, ?, ?, ?)
    ''', flights_data)

users_data = [
        (1, 'user1', 'abc123'),
        (2, 'user2', 'deg456'),
        (3, 'user3', 'abc@123'),
        (4, 'user4', '456hhh'),
        (5, 'user5', '998kkk')
    ]

c.executemany('''
        INSERT INTO Users (user_id, username, password)
        VALUES (?, ?, ?)
    ''', users_data)

    # Inserting data into Tickets table
tickets_data = [
        ('TCKT123',1,'ABC123', 250.00),
        ('TCKT456',2,'DEF456', 200.00),
        ('TCKT789',3,'GHI789', 600.00),
        ('TCKT012',4,'JKL012', 180.00),
        ('TCKT345',5,'MNO345', 300.00)
    ]

c.executemany('''
        INSERT INTO Tickets (ticket_code, user_id, flight_num, ticket_price)
        VALUES (?, ?, ?, ?)
    ''', tickets_data)

conn.commit()
conn.close()