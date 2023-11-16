import random
from datetime import datetime, timedelta

def date():
    start_date = datetime(2022, 1, 1)
    end_date = datetime(2024, 12, 31)
    random_date = start_date + timedelta(days=random.randint(0, (end_date - start_date).days))
    return random_date.strftime("%d/%m/%Y")

def data():
    companies = ["VietNamAirline", "VietJet Air", "Bamboo Airways", "Jetstar Pacific"]
    departure_points = ["HaNoi", "HoChiMinh", "DaNang", "NhaTrang", "PhuQuoc"]
    destination_points = ["QuangNing", "ThanhHoa", "Hue", "Sapa", "ConDao", "NhaTrang", "SaiGon", "HaNoi", "CaoBang", "CaMau", "TruongSa", "NgheAn"]
    seat_classes = ["A", "B", "C", "D"]
    flight_data = []

    for _ in range(100):
        flight_instance = {
            'company': random.choice(companies),
            'flight_num': ''.join(random.choices("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", k=6)),
            'number_of_passenger': random.randint(100, 500),
            'departure_points': random.choice(departure_points),
            'destination_points': random.choice(destination_points),
            'departure_date': date(),
            'return_date': date(),
            'seat_class': random.choice(seat_classes)
        }

        flight_data.append(flight_instance)

    return flight_data

def write_to_file(filename, flight_data):
    with open(filename, 'w') as file:
        for flight_instance in flight_data:
            line = ','.join(str(value) for value in flight_instance.values())
            file.write(f"{line}\n")

if __name__ == "__main__":
    path="project/flight_data.txt"
    data = data()
    write_to_file(path, data)
