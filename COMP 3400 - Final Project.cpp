#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <iomanip>
#include <fstream>
#include <memory>
#include <sstream>
using namespace std;
// utility types required
enum UtilityType {NATURAL_GAS, INTERNET, TV, MOBILE, HOME_PHONE, ELECTRIC, WATER, SEWERAGE};
enum BillStatus {PENDING, PAID, OVERDUE};

class UtilityService {
public:
    string name;
    double baseRate;
    double usageRate;
    UtilityType type;
    
    UtilityService(string n, double base, double usage, UtilityType t) : name(n), baseRate(base), usageRate(usage), type(t) {}   
    double calculateBill(double usage) {return baseRate + (usage * usageRate);}
};

// provider class - each of 10 providers offers all utility services
class Provider {
public:
    int id;
    string name;
    vector<UtilityService> services;
    
    Provider(int i, string n) : id(i), name(n) {
        services = {
            {"Natural Gas", 15.0, 0.05, NATURAL_GAS},
            {"Internet", 10.0, 0.0, INTERNET},
            {"TV", 25.0, 0.0, TV},
            {"Mobile", 15.0, 0.02, MOBILE},
            {"Home Phone", 10.0, 0.01, HOME_PHONE},
            {"Electric", 20.0, 0.10, ELECTRIC},
            {"Water", 12.0, 0.05, WATER},
            {"Sewerage", 8.0, 0.03, SEWERAGE}
        };
    }
};

// customer class - supports customers choosing services from different providers
class Customer {
public:
    int id;
    string name;
    struct ServiceUsage {
        int providerID;
        UtilityType serviceType;
        double usage;
    };
    vector<ServiceUsage> subscriptions; 
    Customer(int i, string n) : id(i), name(n) {}
    
    void addService(int providerID, UtilityType type, double usage) {
        subscriptions.push_back({providerID, type, usage});
    }
};

// bill class - the billing system with payment tracking
class Bill {
public:
    int id;
    int customerID;
    double amount;
    string dueDate;
    BillStatus status;
    
    Bill(int i, int cid, double a, string date) 
        : id(i), customerID(cid), amount(a), dueDate(date), status(PENDING) {}
};

class UtilitySystem {
private:
    vector<Provider> providers;
    vector<Customer> customers;
    vector<Bill> bills;
    int billCounter = 1;
    
    string getCurrentDate() {
        time_t now = time(0);
        tm* now_tm = localtime(&now);
        char date[11];
        strftime(date, 11, "%Y-%m-%d", now_tm);
        return string(date);
    }
    
public:
    UtilitySystem() {
        for (int i = 1; i <= 10; i++) {
            providers.push_back(Provider(i, "Provider " + to_string(i)));
        }
    }
    
    void addCustomer(int id, string name) {
        customers.push_back(Customer(id, name));
        cout << "Added customer: " << name << " (ID: " << id << ")" << endl;
    }
    
    void assignService(int customerId, int providerId, int serviceTypeInt, double usage) {
        UtilityType serviceType = static_cast<UtilityType>(serviceTypeInt);
        for (auto& customer : customers) {
            if (customer.id == customerId) {
                customer.addService(providerId, serviceType, usage);
                cout << "Service assigned to customer ID " << customerId << endl;
                return;
            }
        }
        cout << "Customer not found :(" << endl;
    }
    
    double calculateBill(int customerId) {
        double total = 0.0;
        for (auto& customer : customers) {
            if (customer.id == customerId) {
                for (auto& sub : customer.subscriptions) {
                    auto& provider = providers[sub.providerID - 1];
                    auto& service = provider.services[sub.serviceType];
                    total += service.calculateBill(sub.usage);
                }
                return total;
            }
        }
        return 0.0;
    }
    
    void generateBill(int customerId) {
        double amount = calculateBill(customerId);
        if (amount > 0) {
            string date = getCurrentDate();
            bills.push_back(Bill(billCounter++, customerId, amount, date));
            cout << "Bill generated: $" << amount << " for customer ID " << customerId << endl;
            // This is to save bill to file
            ofstream file("bills.txt", ios::app);
            file << bills.back().id << "," << customerId << "," << amount << "," << date << ",PENDING" << endl;
            file.close();
        } else {
            cout << "Customer not found/having no services :(" << endl;
        }
    }
    
    void displayBills() {
        cout << "Bills:" << endl;
        for (auto& bill : bills) {
            string statusStr;
            switch (bill.status) {
                case PAID: statusStr = "PAID"; break;
                case OVERDUE: statusStr = "OVERDUE"; break;
                default: statusStr = "PENDING";
            }
            
            cout << "Bill ID: " << bill.id 
                 << " | Customer ID: " << bill.customerID 
                 << " | Amount: $" << bill.amount 
                 << " | Due: " << bill.dueDate 
                 << " | Status: " << statusStr << endl;
        }
    }
    
    void checkOverdueBills() {
        time_t now = time(0);
        for (auto& bill : bills) {
            if (bill.status == PENDING) {
                tm due_tm = {};
                istringstream ss(bill.dueDate);
                ss >> get_time(&due_tm, "%Y-%m-%d");
                time_t due_time = mktime(&due_tm);
                
                double days = difftime(now, due_time) / (60 * 60 * 24);
                if (days > 30) {
                    bill.status = OVERDUE;
                    cout << "Bill ID " << bill.id << " is now OVERDUE =((" << endl;
                }
            }
        }
    }
    
    void payBill(int billId) {
        for (auto& bill : bills) {
            if (bill.id == billId) {
                bill.status = PAID;
                cout << "Bill ID " << billId << " marked as PAID =))" << endl;
                return;
            }
        }
        cout << "Bill not found :(" << endl;
    }
};

// menu function
void showMenu(UtilitySystem& system) {
    int choice, customerId, providerId, serviceType, billId;
    string name;
    double usage;
    
    while (true) {
        cout << "\n=== Utility Service System ===" << endl;
        cout << "1. Add Customer\n2. Assign Service\n3. Generate Bill" << endl;
        cout << "4. View Bills\n5. Check Overdue\n6. Pay Bill\n7. Exit" << endl;
        cout << "Choice: ";
        cin >> choice;     
        switch (choice) {
            case 1:
                cout << "Customer ID: "; cin >> customerId;
                cout << "Name: "; cin.ignore(); getline(cin, name);
                system.addCustomer(customerId, name);
                break;
            case 2:
                cout << "Customer ID: "; cin >> customerId;
                cout << "Provider ID (1-10): "; cin >> providerId;
                cout << "Service Type (0-7): "; cin >> serviceType;
                cout << "Usage: "; cin >> usage;
                system.assignService(customerId, providerId, serviceType, usage);
                break;
            case 3:
                cout << "Customer ID: "; cin >> customerId;
                system.generateBill(customerId);
                break;
            case 4:
                system.displayBills();
                break;
            case 5:
                system.checkOverdueBills();
                break;
            case 6:
                cout << "Bill ID: "; cin >> billId;
                system.payBill(billId);
                break;
            case 7:
                cout << "Exiting" << endl;
                return;
            default:
                cout << "Invalid choice" << endl;
        }
    }
}

int main() {
    UtilitySystem system;
    cout << "Welcome to Utility Service Provider System" << endl;
    cout << "----------------------------------------" << endl;
    showMenu(system);
    return 0;
}