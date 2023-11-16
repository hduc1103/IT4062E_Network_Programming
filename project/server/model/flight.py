class flight:
    def __init__(self, company, flight_num, number_of_passenger, departure_points, destination_points, departure_date, return_date, seat_class):
        self.company = company
        self.flight_num = flight_num
        self.number_of_passenger = number_of_passenger
        self.departure_points = departure_points
        self.destination_points = destination_points
        self.departure_date = departure_date
        self.return_date = return_date
        self.seat_class = seat_class

    @staticmethod
    def add_flight(flight_list, company, flight_num, number_of_passenger, departure_points, destination_points, departure_date, return_date, seat_class):
        new_flight = flight(company, flight_num, number_of_passenger, departure_points, destination_points, departure_date, return_date, seat_class)
        flight_list.append(new_flight)

    @staticmethod
    def delete_flight(flight_list, flight_num):
        for flight_instance in flight_list:
            if flight_instance.flight_num == flight_num:
                flight_list.remove(flight_instance)
                print(f"Flight {flight_num} deleted successfully.")
                return
        print(f"Flight {flight_num} not found.")



    """def display_flight_details(self):
        print(
            f"Flight {self.number_of_passenger} from {self.departure_points} to {self.destination_points}")
        print(f"Departure time: {self.departure_date}")
        print(f"Arrival time: {self.return_date}")
        print(f"Company:{self.company} flight number: {self.flight_num}")
        print(f"Flight number: {self.flight_num}")


flight1 = flight("VietName Airlines", "HN141", "123", "HaNoi",
                 "HCM", "2023-11-09", "2023-11-09", "A")
flight1.display_flight_details() 
"""
