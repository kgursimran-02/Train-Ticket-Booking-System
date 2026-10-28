#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <limits>
using namespace std;

class Train {
private:
    int number, capacity, fare;
    string name, source, destination;
public:
    Train(int n, string nm, string s, string d, int seats, int f)
        : number(n), capacity(seats), fare(f), name(nm), source(s), destination(d) {}
    int getNumber() const { return number; }
    int getCapacity() const { return capacity; }
    int getFare() const { return fare; }
    void display(int available) const {
        cout << left << setw(9) << number << setw(19) << name
             << setw(15) << source << setw(15) << destination
             << setw(9) << fare << available << '\n';
    }
};

class Booking {
private:
    int pnr, trainNumber, age, seats;
    string passengerName;
public:
    Booking(int p, int t, string name, int a, int s)
        : pnr(p), trainNumber(t), age(a), seats(s), passengerName(name) {}
    int getPNR() const { return pnr; }
    int getTrainNumber() const { return trainNumber; }
    int getSeats() const { return seats; }
    void display(int fare) const {
        cout << "\n----- TICKET -----\nPNR: " << pnr
             << "\nPassenger: " << passengerName << "\nAge: " << age
             << "\nTrain Number: " << trainNumber
             << "\nSeats: " << seats
             << "\nTotal Fare: Rs. " << seats * fare
             << "\n------------------\n";
    }
    string serialize() const {
        return to_string(pnr) + "|" + to_string(trainNumber) + "|" +
               passengerName + "|" + to_string(age) + "|" + to_string(seats);
    }
    static bool parse(const string& line, Booking& result) {
        stringstream ss(line);
        string fields[5];
        for (int i = 0; i < 5; ++i)
            if (!getline(ss, fields[i], '|')) return false;
        try {
            size_t pos;
            int p = stoi(fields[0], &pos); if (pos != fields[0].size()) return false;
            int t = stoi(fields[1], &pos); if (pos != fields[1].size()) return false;
            int a = stoi(fields[3], &pos); if (pos != fields[3].size()) return false;
            int s = stoi(fields[4], &pos); if (pos != fields[4].size()) return false;
            if (p < 1001 || a < 1 || a > 120 || s < 1 || fields[2].empty()) return false;
            result = Booking(p, t, fields[2], a, s);
            return true;
        } catch (...) { return false; }
    }
};

class RailwaySystem {
private:
    vector<Train> trains;
    vector<Booking> bookings;
    int nextPNR = 1001;
    const string bookingFile = "bookings.txt";
    const string counterFile = "next_pnr.txt";

    static int readInt(const string& prompt, int minimum, int maximum) {
        int value;
        while (true) {
            cout << prompt;
            if (cin >> value && value >= minimum && value <= maximum) {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                return value;
            }
            if (cin.eof()) { cout << "\nInput closed.\n"; exit(0); }
            cout << "Invalid input. Try again.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
    const Train* findTrain(int number) const {
        for (const auto& train : trains)
            if (train.getNumber() == number) return &train;
        return nullptr;
    }
    int availableSeats(int number) const {
        const Train* train = findTrain(number);
        if (!train) return 0;
        int remaining = train->getCapacity();
        for (const auto& b : bookings)
            if (b.getTrainNumber() == number) remaining -= b.getSeats();
        return remaining;
    }
    bool saveBookings() const {
        ofstream out(bookingFile, ios::trunc);
        if (!out) return false;
        for (const auto& b : bookings) out << b.serialize() << '\n';
        out.flush();
        return out.good();
    }
    bool saveCounter() const {
        ofstream out(counterFile, ios::trunc);
        if (!out) return false;
        out << nextPNR << '\n';
        out.flush();
        return out.good();
    }
    void loadData() {
        ifstream in(bookingFile);
        string line;
        while (getline(in, line)) {
            Booking b(0, 0, "", 1, 1);
            if (Booking::parse(line, b) && findTrain(b.getTrainNumber())) {
                bool duplicate = false;
                for (const auto& existing : bookings)
                    if (existing.getPNR() == b.getPNR()) duplicate = true;
                if (!duplicate && availableSeats(b.getTrainNumber()) >= b.getSeats()) {
                    bookings.push_back(b);
                    if (b.getPNR() >= nextPNR) nextPNR = b.getPNR() + 1;
                }
            }
        }
        ifstream counter(counterFile);
        int saved;
        if (counter >> saved && saved > nextPNR) nextPNR = saved;
    }
public:
    RailwaySystem() {
        trains.emplace_back(12001, "Shatabdi Express", "Delhi", "Chandigarh", 50, 500);
        trains.emplace_back(12002, "Punjab Express", "Amritsar", "Delhi", 60, 450);
        trains.emplace_back(12003, "Himalayan Express", "Chandigarh", "Shimla", 40, 300);
        trains.emplace_back(12004, "Intercity Express", "Ludhiana", "Amritsar", 45, 250);
        loadData();
    }
    void showTrains() const {
        cout << "\n---------------- AVAILABLE TRAINS ----------------\n";
        cout << left << setw(9) << "Number" << setw(19) << "Name"
             << setw(15) << "From" << setw(15) << "To"
             << setw(9) << "Fare(Rs)" << "Seats\n";
        for (const auto& t : trains) t.display(availableSeats(t.getNumber()));
    }
    void bookTicket() {
        showTrains();
        int number = readInt("Enter train number: ", 1, 999999);
        const Train* train = findTrain(number);
        if (!train) { cout << "Train not found.\n"; return; }
        int available = availableSeats(number);
        if (available == 0) { cout << "No seats available.\n"; return; }
        string name;
        do {
            cout << "Enter passenger name: ";
            getline(cin, name);
            if (cin.eof()) { cout << "\nInput closed.\n"; exit(0); }
            if (name.empty() || name.find('|') != string::npos)
                cout << "Name cannot be empty or contain '|'.\n";
        } while (name.empty() || name.find('|') != string::npos);
        int age = readInt("Enter passenger age: ", 1, 120);
        int seats = readInt("Enter seats to book (1-" + to_string(available) + "): ", 1, available);
        Booking booking(nextPNR, number, name, age, seats);
        bookings.push_back(booking);
        if (!saveBookings()) {
            bookings.pop_back();
            cout << "Booking failed: could not save file.\n";
            return;
        }
        ++nextPNR;
        if (!saveCounter()) cout << "Warning: could not save PNR counter.\n";
        cout << "\nTicket booked successfully!\n";
        booking.display(train->getFare());
    }
    void viewTicket() const {
        int pnr = readInt("Enter PNR number: ", 1, numeric_limits<int>::max());
        for (const auto& b : bookings)
            if (b.getPNR() == pnr) {
                const Train* train = findTrain(b.getTrainNumber());
                b.display(train->getFare());
                return;
            }
        cout << "PNR not found.\n";
    }
    void cancelTicket() {
        int pnr = readInt("Enter PNR to cancel: ", 1, numeric_limits<int>::max());
        for (size_t i = 0; i < bookings.size(); ++i) {
            if (bookings[i].getPNR() == pnr) {
                Booking old = bookings[i];
                bookings.erase(bookings.begin() + i);
                if (!saveBookings()) {
                    bookings.insert(bookings.begin() + i, old);
                    cout << "Cancellation failed: could not save file.\n";
                    return;
                }
                cout << "Ticket cancelled successfully. Seats restored.\n";
                return;
            }
        }
        cout << "PNR not found.\n";
    }
    void showAllBookings() const {
        if (bookings.empty()) { cout << "No active bookings.\n"; return; }
        for (const auto& b : bookings) {
            const Train* t = findTrain(b.getTrainNumber());
            b.display(t->getFare());
        }
    }
    void run() {
        while (true) {
            cout << "\n===== TRAIN TICKET BOOKING SYSTEM =====\n"
                 << "1. View Available Trains\n2. Book Ticket\n"
                 << "3. View Ticket by PNR\n4. Cancel Ticket\n"
                 << "5. Check Seat Availability\n6. View All Bookings\n7. Exit\n";
            int choice = readInt("Enter your choice: ", 1, 7);
            switch (choice) {
                case 1: showTrains(); break;
                case 2: bookTicket(); break;
                case 3: viewTicket(); break;
                case 4: cancelTicket(); break;
                case 5: {
                    int number = readInt("Enter train number: ", 1, 999999);
                    if (!findTrain(number)) cout << "Train not found.\n";
                    else cout << "Available seats: " << availableSeats(number) << '\n';
                    break;
                }
                case 6: showAllBookings(); break;
                case 7: cout << "Thank you for using the system!\n"; return;
            }
        }
    }
};

int main() {
    RailwaySystem system;
    system.run();
    return 0;
}
