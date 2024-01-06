#include <iostream>
#include <sqlite3.h>
#include <vector>
#include <tuple>

using namespace std;

int main()
{
    sqlite3 *conn;
    int rc;

    rc = sqlite3_open("flight_database.db", &conn);
    if (rc)
    {
        cerr << "Can't open database: " << sqlite3_errmsg(conn) << endl;
        return 1;
    }

    sqlite3_stmt *stmt;

    const char *createFlightsTableSQL = R"(
        CREATE TABLE IF NOT EXISTS Flights (
            flight_num VARCHAR(20) PRIMARY KEY,
            number_of_passenger INT,
            departure_point VARCHAR(50),
            destination_point VARCHAR(50),
            departure_date DATE,
            return_date DATE
        )
    )";

    rc = sqlite3_exec(conn, createFlightsTableSQL, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK)
    {
        cerr << "SQL error: " << sqlite3_errmsg(conn) << endl;
        sqlite3_close(conn);
        return 1;
    }

    const char *insertFlightsSQL = "INSERT INTO Flights (flight_num, number_of_passenger, departure_point, destination_point, departure_date, return_date) VALUES (?, ?, ?, ?, ?, ?)";

    vector<tuple<string, int, string, string, string, string>> flights_data = {
        {"ABC123", 150, "CaMau", "Vinh", "2023-01-15", "2023-01-20"},
        {"DEF456", 200, "HaNoi", "HoChiMinh", "2023-02-10", "2023-02-15"},
        {"GHI789", 100, "NgheAn", "CaoBang", "2023-03-05", "2023-03-10"},
        {"JKL012", 180, "QuyNhon", "HaiPhong", "2023-04-20", "2023-04-25"},
        {"MNO345", 120, "ThanhHoa", "KhanhHoa", "2023-05-12", "2023-05-18"}};

    for (const auto &row : flights_data)
    {
        rc = sqlite3_prepare_v2(conn, insertFlightsSQL, -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            cerr << "SQL error: " << sqlite3_errmsg(conn) << endl;
            sqlite3_close(conn);
            return 1;
        }

        sqlite3_bind_text(stmt, 1, get<0>(row).c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, get<1>(row));
        sqlite3_bind_text(stmt, 3, get<2>(row).c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, get<3>(row).c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, get<4>(row).c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 6, get<5>(row).c_str(), -1, SQLITE_STATIC);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            cerr << "SQL error: " << sqlite3_errmsg(conn) << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(conn);
            return 1;
        }

        sqlite3_finalize(stmt);
    }

    // Create Users table
    const char *createUsersTableSQL = R"(
        CREATE TABLE IF NOT EXISTS Users (
            user_id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT,
            password TEXT
        )
    )";

    rc = sqlite3_exec(conn, createUsersTableSQL, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK)
    {
        cerr << "SQL error: " << sqlite3_errmsg(conn) << endl;
        sqlite3_close(conn);
        return 1;
    }

    const char *insertUsersSQL = "INSERT INTO Users (user_id, username, password) VALUES (?, ?, ?)";

    vector<tuple<int, string, string>> users_data = {
        {1, "user1", "abc123"},
        {2, "user2", "deg456"},
        {3, "user3", "abc@123"},
        {4, "user4", "456hhh"},
        {5, "user5", "998kkk"}};

    for (const auto &row : users_data)
    {
        rc = sqlite3_prepare_v2(conn, insertUsersSQL, -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            cerr << "SQL error: " << sqlite3_errmsg(conn) << endl;
            sqlite3_close(conn);
            return 1;
        }

        sqlite3_bind_int(stmt, 1, get<0>(row));
        sqlite3_bind_text(stmt, 2, get<1>(row).c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, get<2>(row).c_str(), -1, SQLITE_STATIC);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            cerr << "SQL error: " << sqlite3_errmsg(conn) << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(conn);
            return 1;
        }

        sqlite3_finalize(stmt);
    }

    const char *createTicketsTableSQL = R"(
        CREATE TABLE IF NOT EXISTS Tickets (
            ticket_code TEXT,
            user_id INTEGER,
            flight_num VARCHAR(20),
            seat_class TEXT,
            ticket_price REAL,
            FOREIGN KEY (user_id) REFERENCES Users(user_id),
            FOREIGN KEY (flight_num) REFERENCES Flights(flight_num)
        )
    )";

    rc = sqlite3_exec(conn, createTicketsTableSQL, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK)
    {
        cerr << "SQL error: " << sqlite3_errmsg(conn) << endl;
        sqlite3_close(conn);
        return 1;
    }

    const char *insertTicketsSQL = "INSERT INTO Tickets (ticket_code, user_id, flight_num, seat_class,ticket_price) VALUES (?, ?, ?, ?, ?)";

    vector<tuple<string, int, string, string, double>> tickets_data = {
        {"TCKT123", 1, "ABC123", "A", 300.00},
        {"TCKT456", 2, "DEF456", "B", 150.00},
        {"TCKT789", 3, "GHI789", "A", 300.00},
        {"TCKT012", 4, "JKL012", "B", 150.00},
        {"TCKT345", 5, "MNO345", "A", 300.00}
        };

    for (const auto &row : tickets_data)
    {
        rc = sqlite3_prepare_v2(conn, insertTicketsSQL, -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            cerr << "SQL error: " << sqlite3_errmsg(conn) << endl;
            sqlite3_close(conn);
            return 1;
        }

        sqlite3_bind_text(stmt, 1, get<0>(row).c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, get<1>(row));
        sqlite3_bind_text(stmt, 3, get<2>(row).c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt,  4, get<3>(row).c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 5, get<4>(row));

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            cerr << "SQL error: " << sqlite3_errmsg(conn) << endl;
            sqlite3_finalize(stmt);
            sqlite3_close(conn);
            return 1;
        }

        sqlite3_finalize(stmt);
    }
    sqlite3_exec(conn, "COMMIT;", nullptr, nullptr, nullptr);
    sqlite3_close(conn);

    return 0;
}
