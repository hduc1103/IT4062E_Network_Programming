from server.features import *
from server.model import *

from server.model.flight import *
from server.features.search import *

path ="project/flight_data.txt"
def load_data(filename):
    current_list = []
    with open(filename, 'r') as file:
        for line in file:
            data = line.strip().split(',')
            flight.add_flight(current_list, *data)
    return current_list
def main():
    current_list=load_data(path)
#flight_test = [
#    flight("VietNamAirline", "ABC123", 200, "HaNoi", "HoChiMinh", "32/02/2023", "31/05/2023", "B"),
#    flight("VietJet Air", "DEG678", 400, "HaLong", "ThanhHoa", "30/04/2022", "31/06/2022", "A"),
#]

#    for flight_tmp in current_list:
#        flight.add_flight(
#            current_list,
#            flight_tmp.company,
#            flight_tmp.flight_num,
#            flight_tmp.number_of_passenger,
#            flight_tmp.departure_points,
#            flight_tmp.destination_points,
#            flight_tmp.departure_date,
#            flight_tmp.return_date,
#            flight_tmp.seat_class
#        )

    #for flight_tmp in current_list:
    #    print(flight_tmp.company, flight_tmp.flight_num, flight_tmp.seat_class)

    search_criteria = {
        'company' : "VietNamAirline",
        'departure_points': 'NhaTrang',
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