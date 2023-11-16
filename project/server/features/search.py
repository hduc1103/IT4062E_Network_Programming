from typing import List
from server.model.flight import flight

def search_flights(flight_list: List[flight], criteria: dict):
    result_flights = []

    for flight_instance in flight_list:
        match = True

        for key, value in criteria.items():
            if getattr(flight_instance, key, None) != value:
                match = False
                break

        if match:
            result_flights.append(flight_instance)

    return result_flights
