from server.features import *
from server.model import *

from server.model.flight import *
from server.features.search import *

path ="server/flight_data.txt"
def load_data(filename):
    current_list = []
    with open(filename, 'r') as file:
        for line in file:
            data = line.strip().split(',')
            flight.add_flight(current_list, *data)
    return current_list
def main():
    current_list=load_data(path)
    search_criteria = {
        'company' : "VietNamAirline",
        'departure_points': 'DaNang',
        'seat_class': 'C'
    }

    res = search_flights(current_list, search_criteria)

    print("Search Results:")
    if res is not None:
        for flight_instance in res:
            print(flight_instance.company, flight_instance.flight_num, flight_instance.seat_class)
    else:
        print("NotFound")

if __name__ == "__main__":
    main()
