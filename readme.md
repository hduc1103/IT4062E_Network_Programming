```markdown
# Flight Ticket Booking System

## Overview
This application allows users to search, compare, and book flight tickets online from various airlines. Users can manage their bookings, cancel or change tickets, and receive flight notifications. The application uses SQLite for data storage, C++ for backend logic, and sockets for client-server communication.

---

## Features

### User Features
- **Account Management**: Register and log in to the system.
- **Flight Search**: Search for flights based on departure and destination, dates, number of passengers, and seat class.
- **Ticket Comparison**: Compare ticket prices and flight times from different airlines.
- **Booking and Payment**: Book flights and make payments via credit card or electronic wallets.
- **Electronic Tickets**: Receive electronic ticket codes via email or text.
- **Booking Management**: View, cancel, change, and print tickets.
- **Flight Notifications**: Get notified of schedule changes, cancellations, and delays.

### Admin Features
- **Flight Management**: Add, update, or cancel flights.
- **Notification Management**: Send notifications for schedule changes, cancellations, and delays.

---

## Assignment Grading Criteria
The system implements the following functionalities as per teacher requirements:
- **Client-Server Connection**: Establish connection using sockets (2 points).
- **Server Logging**: Log messages on the server (1 point).
- **User Registration and Login**: Create and authenticate user accounts (2 points).
- **Flight Search**: Search flights with various criteria (2 points).
- **Ticket Comparison**: Compare ticket prices and flight times (2 points).
- **Booking and Payment**: Book flights and process payments online (3 points).
- **Electronic Ticketing**: Send ticket codes via email or text (2 points).
- **Booking Management**: View details, cancel, change, and print tickets (3 points).
- **Notifications**: Notify users of changes, delays, and cancellations (3 points).

---

## Getting Started

### Prerequisites
- Linux environment
- `sqlite3` installed

### Installation
1. Clone the repository:
   ```bash
   git clone <your-repo-url>
   ```
2. Navigate to the project directory:
   ```bash
   cd <project-directory>
   ```
3. Compile the C++ code:
   ```bash
   g++ -o server server.cpp -lsqlite3
   g++ -o client client.cpp
   ```
4. Set up the SQLite database:
   - Open the SQLite shell:
     ```bash
     sqlite3 flight_booking.db
     ```
   - Run the provided SQL script to create tables:
     ```sql
     .read setup.sql
     ```
5. Start the server:
   ```bash
   ./server
   ```
6. Connect the client:
   ```bash
   ./client
   ```

---

## Security
- User passwords are securely hashed before being stored in the database.
- All sensitive data, including payment details, is transmitted over encrypted connections (e.g., TLS/SSL).
- SQL queries use parameterized statements to prevent SQL injection.

---

## Testing and Validation
- Unit tests are provided for critical functionalities such as:
  - User account management.
  - Flight search and booking.
- Integration tests ensure seamless communication between the client and server.
- Simulated real-world scenarios validate the reliability of the system.

---

## Technologies Used
- **Programming Language**: C++
- **Database**: SQLite
- **Communication**: Sockets (TCP)
- **Operating System**: Linux

---
```
