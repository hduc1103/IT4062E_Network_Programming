import sqlite3

conn = sqlite3.connect('./server/flight_database.db')
cursor = conn.cursor()

# cursor.execute('DROP TABLE flights')
cursor.execute('SELECT * FROM flights')
flight_data = cursor.fetchall()
print("Flight Table Data:")
for row in flight_data:
    print(row)
search_criteria = {
                    'flight_num': "XLK753"
                }
sql = "SELECT * FROM flights WHERE flight_num = ?"
params = [search_criteria.get('flight_num')]
print(params)
cursor.execute(sql, params)
res = cursor.fetchall()
print("Search Results:\n")
if res:
            for tmp_flight in res:
                result_str = f"{tmp_flight[0]},{tmp_flight[1]},{tmp_flight[7]};"
                result_str = "Y_found/" + result_str
                print(result_str)
else:
    print("N")
# Close the connection
conn.close()
