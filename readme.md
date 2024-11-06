```markdown
# Flight Ticket Booking System

## Overview
This application allows users to search, compare, and book flight tickets online from various airlines. Users can manage their bookings, cancel or change tickets, and receive flight notifications. The application uses SQLite for data storage, C++ for backend logic, and sockets for client-server communication.

## Features
### User Features
- **Account Management**: Register and log in to the system.
- **Flight Search**: Search for flights based on departure and destination, dates, number of passengers, and seat class.
- **Ticket Comparison**: Compare ticket prices and flight times from different airlines.
- **Booking and Payment**: Book flights and make payments via credit card or electronic wallets.
- **Electronic Tickets**: Receive electronic ticket codes via email or text.
- **Booking Management**: View, cancel, change, and print tickets.
- **Flight Notifications**: Get notified of schedule changes, cancellations, and delays.

### Teacher Requirements
- Establish client-server connection using sockets (2 points).
- Log messages on the server (1 point).
- User account registration and login (2 points).
- Flight search with various criteria (2 points).
- Compare ticket prices and flight times (2 points).
- Book flights and make online payments (3 points).
- Send electronic ticket codes via email or text (2 points).
- Manage bookings: view details, cancel, change, print tickets (3 points).
- Receive flight notifications (3 points). *Admin feature*

## Getting Started

### Prerequisites
- Linux environment
- `sqlite3` installed

### Installation
1. Clone the repository:
   ```bash
   git clone <your-repo-url>
   ```
2. Build the application:
   ```bash
   make all
   ```

### Running the Application
1. Start the server:
   ```bash
   ./server
   ```
2. Start the client:
   ```bash
   ./client
   ```

## Architecture
The application follows a client-server model:
- **Server**: Handles all user requests, manages bookings, sends notifications, and logs received/sent messages.
- **Client**: Provides a user interface for account management, flight search, booking, and ticket management.

## Database
- Uses SQLite to store user accounts, bookings, flight details, and payment information securely.

## Future Enhancements
- Improve error handling and security.
- Enhance UI for a more seamless user experience.
- Add support for more payment methods.

---

Thank you for reviewing my project! A ⭐️ would be much appreciated if you found this helpful. 😊
``` 
