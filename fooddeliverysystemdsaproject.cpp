#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <list>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <regex> 
#include <cstdlib>  
#include <ctime>
#include <conio.h>
#include <windows.h>
#include <fstream>
#include <sstream>
#include <map>
using namespace std;

void setcolor(int color){
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

class MenuItem;
class Order;
class Restaurant;
class DeliveryDriver;
class Address;
class Payment;

class Address {
private:
    string streetAddress;
    string city;
    string zipCode;
public:
    Address(string street, string c, string zip)
        : streetAddress(street), city(c), zipCode(zip) {}
    
    string getFullAddress() const {
        return streetAddress + ", " + city +  " " + zipCode;
    }

    // For serialization
    string serialize() const {
        return streetAddress + "|" + city +  "|" + zipCode;
    }

    static Address* deserialize(const string& data) {
        stringstream ss(data);
        string street, city, zip;
        getline(ss, street, '|');
        getline(ss, city, '|');
        getline(ss, zip);
        return new Address(street, city, zip);
    }
};

class Payment {
private:
    string cardNumber;
    string cardType;
    string expiryDate;
    string cvv;
public:
    Payment(string num, string type, string exp, string cv)
        : cardNumber(num), cardType(type), expiryDate(exp), cvv(cv) {}   
    static bool validateCardNumber(const string& number) {
        string cleaned = "";
        for (char c : number) {
            if (isdigit(c)) cleaned += c;
        }
        if (cleaned.length() != 16) return false;
        int sum = 0;
        bool alternate = false;
        for (int i = cleaned.length() - 1; i >= 0; i--) {
            int n = cleaned[i] - '0';
            if (alternate) {
                n *= 2;
                if (n > 9) n -= 9;
            }
            sum += n;
            alternate = !alternate;
        }
        return (sum % 10 == 0);
    }
    static string detectCardType(const string& number) {
        if (number[0] == '4') return "Visa";
        if (number[0] == '5') return "MasterCard";
        if (number[0] == '3' && (number[1] == '4' || number[1] == '7')) return "American Express";
        if (number[0] == '6') return "Discover";
        return "Unknown";
    }
    string getMaskedCardNumber() const {
        return "****-****-****-" + cardNumber.substr(12);
    }
    // For serialization
    string serialize() const {
        // Store cardNumber, cardType, expiryDate, cvv separated by '|'
        return cardNumber + "|" + cardType + "|" + expiryDate + "|" + cvv;
    }
    static Payment* deserialize(const string& data) {
        stringstream ss(data);
        string num, type, exp, cv;
        getline(ss, num, '|');
        getline(ss, type, '|');
        getline(ss, exp, '|');
        getline(ss, cv);
        return new Payment(num, type, exp, cv);
    }
};

class MenuItem {
private:
    string id;
    string name;
    double price;
    string category;
    int preparationTime; 
public:
    MenuItem(string i, string n, double p, string c, int pt)
        : id(i), name(n), price(p), category(c), preparationTime(pt) {}
    string getId() const { return id; }
    string getName() const { return name; }
    double getPrice() const { return price; }
    string getCategory() const { return category; }
    int getPrepTime() const { return preparationTime; }
};
template<typename T>
class BST {
private:
    struct Node {
        T data;
        Node* left;
        Node* right;
        Node(const T& item) : data(item), left(nullptr), right(nullptr) {}
    };
    Node* root;
    Node* insert(Node* node, const T& item) {
        if (!node) return new Node(item);
        if (item->getPrice() < node->data->getPrice())
            node->left = insert(node->left, item);
        else
            node->right = insert(node->right, item);
        return node;
    }
    void inorderTraversal(Node* node, vector<T>& items) const {
        if (!node) return;
        inorderTraversal(node->left, items);
        items.push_back(node->data);
        inorderTraversal(node->right, items);
    }
public:
    BST() : root(nullptr) {}
    void insert(const T& item) {
        root = insert(root, item);
    }
    vector<T> getInorder() const {
        vector<T> items;
        inorderTraversal(root, items);
        return items;
    }
};
template<typename T>
class OrderQueue {
private:
    struct Node {
        T data;
        int priority;
        Node* next;
        Node(T d, int p) : data(d), priority(p), next(nullptr) {}
    };
    Node* head; 
public:
    OrderQueue() : head(nullptr) {}
    void push(T item, int priority) {
        Node* newNode = new Node(item, priority);
        
        if (!head || priority > head->priority) {
            newNode->next = head;
            head = newNode;
        } else {
            Node* current = head;
            while (current->next && current->next->priority >= priority) {
                current = current->next;
            }
            newNode->next = current->next;
            current->next = newNode;
        }
    }
    T pop() {
        if (!head) throw runtime_error("Queue is empty");
        Node* temp = head;
        T item = temp->data;
        head = head->next;
        delete temp;
        return item;
    }
    bool empty() const { return head == nullptr; }
};

class Order {
private:
    string orderId;
    vector<MenuItem*> items;
    double totalAmount;
    string customerName;
    string status;
    chrono::system_clock::time_point orderTime;
    Address* deliveryAddress;
    Payment* paymentInfo;
    string phoneNumber;
    string specialInstructions;
public:
    Order(string id, string name, Address* addr, Payment* payment, string phone) 
        : orderId(id), customerName(name), totalAmount(0.0), 
          status("Pending"), orderTime(chrono::system_clock::now()),
          deliveryAddress(addr), paymentInfo(payment), phoneNumber(phone) {}
    
    void addItem(MenuItem* item) {
        items.push_back(item);
        totalAmount += item->getPrice();
    }
    string getId() const { return orderId; }
    double getTotal() const { return totalAmount; }
    string getStatus() const { return status; }
    void setStatus(string s) { status = s; }
    string getCustomerName() const { return customerName; }
    const vector<MenuItem*>& getItems() const { return items; }
    void setSpecialInstructions(const string& instructions) {
        specialInstructions = instructions;
    }
    string getDeliveryAddress() const {
        return deliveryAddress ? deliveryAddress->getFullAddress() : "No address provided";
    }
    string getPaymentInfo() const {
        return paymentInfo ? paymentInfo->getMaskedCardNumber() : "No payment info";
    }
    string getPhoneNumber() const { return phoneNumber; }
    string getSpecialInstructions() const { return specialInstructions; }

    // Serialization for file
    string serialize() const {
        stringstream ss;
        ss << orderId << ",";
        ss << customerName << ",";
        ss << phoneNumber << ",";
        ss << deliveryAddress->serialize() << ",";
        ss << paymentInfo->serialize() << ",";
        ss << fixed << setprecision(2) << totalAmount << ",";
        ss << status << ",";
        // serialize items as semicolon separated item ids
        for (size_t i = 0; i < items.size(); ++i) {
            ss << items[i]->getId();
            if (i != items.size() - 1) ss << ";";
        }
        ss << ",";
        // serialize special instructions, escape commas by replacing them with \,
        string escapedInstructions = specialInstructions;
        size_t pos = 0;
        while ((pos = escapedInstructions.find(',', pos)) != string::npos) {
            escapedInstructions.replace(pos, 1, "\\,");
            pos += 2;
        }
        ss << escapedInstructions;
        return ss.str();
    }

    static Order* deserialize(const string& data, const map<string, MenuItem*>& menuItems) {
        stringstream ss(data);
        string orderId, customerName, phoneNumber, addressStr, paymentStr, totalStr, status, itemsStr, specialInstr;
        getline(ss, orderId, ',');
        getline(ss, customerName, ',');
        getline(ss, phoneNumber, ',');
        getline(ss, addressStr, ',');
        getline(ss, paymentStr, ',');
        getline(ss, totalStr, ',');
        getline(ss, status, ',');
        getline(ss, itemsStr, ',');
        getline(ss, specialInstr);

        Address* address = Address::deserialize(addressStr);
        Payment* payment = Payment::deserialize(paymentStr);
        Order* order = new Order(orderId, customerName, address, payment, phoneNumber);
        order->setStatus(status);

        // Convert totalStr to double and set totalAmount directly (to avoid double adding)
        double totalAmount = 0.0;
        try {
            totalAmount = stod(totalStr);
        } catch(...) {}
        order->totalAmount = totalAmount;

        // Parse item ids and resolve MenuItem pointers
        stringstream ssItems(itemsStr);
        string itemId;
        while (getline(ssItems, itemId, ';')) {
            if (menuItems.count(itemId)) {
                order->items.push_back(menuItems.at(itemId));
            }
        }
        // Special instructions unescape
        size_t pos = 0;
        while ((pos = specialInstr.find("\\,", pos)) != string::npos) {
            specialInstr.replace(pos, 2, ",");
            pos += 1;
        }
        order->setSpecialInstructions(specialInstr);

        return order;
    }

    ~Order() {
        delete deliveryAddress;
        delete paymentInfo;
    }
};

class Restaurant {
private:
    void quickSort(vector<Order*>& orders, int low, int high) {
        if (low < high) {
            // Use median-of-three pivot selection
            int mid = low + (high - low) / 2;
            if (orders[low]->getTotal() > orders[mid]->getTotal()) 
                swap(orders[low], orders[mid]);
            if (orders[mid]->getTotal() > orders[high]->getTotal())
                swap(orders[mid], orders[high]);
            if (orders[low]->getTotal() > orders[mid]->getTotal())
                swap(orders[low], orders[mid]);
            
            double pivot = orders[mid]->getTotal();
            int i = low - 1;
            int j = high + 1;
            
            while (true) {
                do {
                    i++;
                } while (orders[i]->getTotal() < pivot);
                
                do {
                    j--;
                } while (orders[j]->getTotal() > pivot);
                
                if (i >= j) break;
                
                swap(orders[i], orders[j]);
            }
            
            quickSort(orders, low, j);
            quickSort(orders, j + 1, high);
        }
    }

    void mergeSort(vector<Order*>& orders, int left, int right) {
        if (left < right) {
            int mid = left + (right - left) / 2;
            mergeSort(orders, left, mid);
            mergeSort(orders, mid + 1, right);
            merge(orders, left, mid, right);
        }
    }
    void merge(vector<Order*>& orders, int left, int mid, int right) {
        vector<Order*> temp(right - left + 1);
        int i = left, j = mid + 1, k = 0;
        while (i <= mid && j <= right) {
            if (orders[i]->getTotal() <= orders[j]->getTotal())
                temp[k++] = orders[i++];
            else
                temp[k++] = orders[j++];
        }      
        while (i <= mid) temp[k++] = orders[i++];
        while (j <= right) temp[k++] = orders[j++];      
        for (i = 0; i < k; i++)
            orders[left + i] = temp[i];
    }
    Order* binarySearch(const vector<Order*>& orders, const string& id) {
        int left = 0, right = orders.size() - 1;       
        while (left <= right) {
            int mid = left + (right - left) / 2;
            if (orders[mid]->getId() == id)
                return orders[mid];
            if (orders[mid]->getId() < id)
                left = mid + 1;
            else
                right = mid - 1;
        }
        return nullptr;
    }  
    Order* linearSearch(const vector<Order*>& orders, const string& id) {
        for (Order* order : orders) {
            if (order->getId() == id)
                return order;
        }
        return nullptr;
    }

    void saveOrdersToFile(const string& filename) {
        ofstream outFile(filename, ios::trunc); // Use trunc instead of app to avoid duplicates
        if (!outFile) {
            cerr << "Error opening file for writing: " << filename << endl;
            return;
        }
        try {
            for (const auto& pair : orderHistory) {
                outFile << pair.second->serialize() << "\n";
            }
            outFile.close();
            setcolor(10);
            cout << "Orders saved to " << filename << endl;
            setcolor(7);
        } catch (const exception& e) {
            cerr << "Error saving orders: " << e.what() << endl;
            outFile.close();
        }
    }
    void loadOrdersFromFile(const string& filename) {
        ifstream inFile(filename);
        if (!inFile) {
            return;
        }

        try {
            string line;
            orderHistory.clear();
            while (getline(inFile, line)) {
                if (line.empty()) continue;
                Order* order = Order::deserialize(line, menuMap);
                if (order) {
                    orderHistory[order->getId()] = order;
                }
            }
            inFile.close();
        } catch (const exception& e) {
            cerr << "Error loading orders: " << e.what() << endl;
            inFile.close();
        }
    }

public:
    bool validateInput(const string& input, const string& type) {
        if (input.empty()) return false;
        
        if (type == "name") {
            return all_of(input.begin(), input.end(), [](char c) {
                return isalpha(c) || c == ' ' || c == '-';
            });
        }
        else if (type == "zip") {
            return all_of(input.begin(), input.end(), ::isdigit) && input.length() == 5;
        }
        else if (type == "expiry") {
            regex pattern("^(0[1-9]|1[0-2])/([0-9]{2})$");
            return regex_match(input, pattern);
        }
        else if (type == "cvv") {
            return all_of(input.begin(), input.end(), ::isdigit) && input.length() == 3;
        }
        return true;
    }

    BST<MenuItem*> menuTree;
    OrderQueue<Order*> orderQueue;
    unordered_map<string, Order*> orderHistory;
    list<DeliveryDriver*> availableDrivers;
    map<string, MenuItem*> menuMap; // id -> MenuItem
    bool running = true;

    Restaurant() {
        // Initialize RNG for generating order ids
        srand(static_cast<unsigned int>(time(nullptr)));
    }
    void addMenuItem(MenuItem* item) {
        menuTree.insert(item);
        menuMap[item->getId()] = item;
    }  
    void displayMenu() const {
        system("cls");
        vector<MenuItem*> items = menuTree.getInorder();
        setcolor(14); // Yellow
        cout << "\nMenu Items (Sorted by Price):" << endl;
        setcolor(11); // Light cyan
        cout << setw(5) << "ID" 
             << setw(20) << "Name"
             << setw(10) << "Price"
             << setw(15) << "Category"
             << setw(15) << "Prep Time" << endl;
        setcolor(7); // White
        cout << string(65, '-') << endl;       
        for (const MenuItem* item : items) {
            cout << setw(5) << item->getId()
                 << setw(20) << item->getName()
                 << setw(10) << fixed << setprecision(2) << item->getPrice()
                 << setw(15) << item->getCategory()
                 << setw(15) << item->getPrepTime() << " mins" << endl;
        }
    }
    void placeOrder(Order* order) {
        if (!order) return;
        
        system("cls");
        int priority = static_cast<int>(order->getTotal() * 10);
        orderQueue.push(order, priority);
        orderHistory[order->getId()] = order;

        setcolor(10);
        cout << "Order placed successfully. Order ID: " << order->getId() << endl;
        setcolor(7);

        saveOrdersToFile("orders.txt");
    }   
    void displayOrderDetails(const Order* order) {
        system("cls");
        setcolor(11);
        cout << "\nOrder Details:" << endl;
        cout << string(50, '-') << endl;
        cout << "Order ID: " << order->getId() << endl;
        cout << "Customer Name: " << order->getCustomerName() << endl;
        cout << "Phone Number: " << order->getPhoneNumber() << endl;
        cout << "Delivery Address: " << order->getDeliveryAddress() << endl;
        cout << "Payment Method: " << order->getPaymentInfo() << endl;
        setcolor(7);      
        if (!order->getSpecialInstructions().empty()) {
            cout << "Special Instructions: " << order->getSpecialInstructions() << endl;
        }      
        cout << "\nOrdered Items:" << endl;
        for (const MenuItem* item : order->getItems()) {
            cout << "- " << item->getName() << " ($" 
                 << fixed << setprecision(2) << item->getPrice() << ")" << endl;
        }      
        cout << "\nTotal Amount: $" << order->getTotal() << endl;
        cout << "Status: " << order->getStatus() << endl;
    }    
    bool validatePhoneNumber(const string& phone) {
        regex phonePattern(R"(\d{3}-\d{3}-\d{4})");
        return regex_match(phone, phonePattern);
    }
    void trackOrder(const string& orderId) {
        auto it = orderHistory.find(orderId);
        if (it != orderHistory.end()) {
            Order* order = it->second;
            system("cls");
            setcolor(11);
            cout << "\nOrder Tracking Information" << endl; 
            cout << string(50, '-') << endl;
            cout << "Order ID: " << order->getId() << endl;
            cout << "Customer Name: " << order->getCustomerName() << endl;
            cout << "Status: " << order->getStatus() << endl;
            cout << "Delivery Address: " << order->getDeliveryAddress() << endl;
            setcolor(7);
            if (!order->getSpecialInstructions().empty()) {
                cout << "\nSpecial Instructions: " << order->getSpecialInstructions() << endl;
            }
            cout << "\nPress ESC to return to main menu..." << endl;
            int key = _getch();
            if (key == 27) {
                system("cls");
                setcolor(2);
                cout << "Returning to main menu..." << endl;
                setcolor(7);
            }
        } else {
            setcolor(12);
            cout << "\nOrder not found!" << endl;
            cout << "Press ESC to return to main menu or SPACE to try again..." << endl;
            setcolor(7);           
            int key = _getch();
            if (key == 27) {
                system("cls");
                setcolor(2);
                cout << "Returning to main menu..." << endl; 
                setcolor(7);
            } else if (key == 32) {
                system("cls");
                string newOrderId;
                setcolor(15);
                cout << "Enter order ID to track: ";
                setcolor(7);
                cin >> newOrderId;
                trackOrder(newOrderId);
            }
        }
    }
    void displayAllOrders() {
        system("cls");
        loadOrdersFromFile("orders.txt"); // Auto load before displaying
        if(orderHistory.empty()){
            setcolor(12);
            cout << "No orders in history" << endl;
            cout<<"Press ESC to return to main menu."<<endl;
            setcolor(7);
            int key = _getch();
            if(key == 27){
                setcolor(2);
                cout << "Returning to main menu..." << endl;
                setcolor(7);
                running = false;
            } else {
                setcolor(12);
                cout << "Invalid input" << endl;
            }
        } else {
            setcolor(11);
            cout << "Displaying all orders..." << endl;
            setcolor(7);
            for (const auto& pair : orderHistory) {
                cout << "Order ID: " << pair.first << endl;
                cout << "Customer Name: " << pair.second->getCustomerName() << endl;
                cout << "Total Amount: $" << fixed << setprecision(2) << pair.second->getTotal() << endl;
                cout << "Status: " << pair.second->getStatus() << endl;
                cout << "-------------------------------" << endl;
            }
        }
    }
    void searchOrderById() {   
        system("cls");
        loadOrdersFromFile("orders.txt"); // Auto load before searching
        cout << "Searching for order by ID..." << endl;
        string orderId;
        cout << "Enter order ID: ";
        cin >> orderId;
        auto it = orderHistory.find(orderId);
        if (it != orderHistory.end()) {
            displayOrderDetails(it->second);
        }
        else{
            setcolor(12);
            cout << "Order not found, try again." << endl;
            cout << "\nEscape to return to main menu, Space to try again" << endl;
            setcolor(7);
            int key = _getch();
            if(key == 27){
                setcolor(2);
                cout << "Returning to main menu..." << endl;
                setcolor(7);
                running = false;
            }
            else if(key == 32){
                searchOrderById();
            }
            else{
                setcolor(12);
                cout << "Invalid input" << endl;
            }   
        }
    }
    vector<Order*> getOrders() {
        vector<Order*> orders;
        for (const auto& pair : orderHistory) {
            orders.push_back(pair.second);
        }
        return orders;
    }
    void sortOrdersByTotalAmount() {
        system("cls");
        loadOrdersFromFile("orders.txt"); // Auto load before sorting
        cout << "Sorting orders by total amount..." << endl;
        vector<Order*> orders = getOrders();
        sort(orders.begin(), orders.end(), [](Order* a, Order* b) {
            return a->getTotal() < b->getTotal();
        });
        for (Order* order : orders) {
            cout << "Order ID: " << order->getId() << endl;
            cout << "Customer Name: " << order->getCustomerName() << endl;
            cout << "Total Amount: $" << fixed << setprecision(2) << order->getTotal() << endl;
            cout << "Status: " << order->getStatus() << endl;
            cout << "--------------------------" << endl;
        }
    }   
    void viewOrderHistory() {
        system("cls");
        cout << "Order History:" << endl;
        cout << "1. Display all orders" << endl;
        cout << "2. Search for an order by ID" << endl;
        cout << "3. Sort orders by total amount" << endl;
        cout << "Enter your choice: ";
        int choice;
        cin >> choice;
        switch (choice) {
            case 1:
                displayAllOrders();
                break;
            case 2:
                searchOrderById();
                break;
            case 3:
                sortOrdersByTotalAmount();
                break;
            default:
                cout << "Invalid choice. Please try again." << endl;
        }
    }   
    void processOrders() {
        while (!orderQueue.empty()) {
            Order* order = orderQueue.pop();
            processOrders();
            cout << "Processing order: " << order->getId() << endl;
            cout << "Customer Name: " << order->getCustomerName() << endl;
            cout << "Total Amount: $" << fixed << setprecision(2) << order->getTotal() << endl;
            cout << "Status: " << order->getStatus() << endl;
            cout << "--------------------------" << endl;
            order->setStatus("Completed");
            cout << "Order completed: " << order->getId() << endl;
            cout << "--------------------------" << endl;
        }
    }
    
    ~Restaurant() {
        // Clean up menu items
        for (const auto& pair : menuMap) {
            delete pair.second;
        }
        // Clean up orders
        for (const auto& pair : orderHistory) {
            delete pair.second;
        }
        // Clean up drivers
        for (auto driver : availableDrivers) {
            delete driver;
        }
    }
};

void showtitle(){
    setcolor(7);
    cout<<"==Welcome to the Food Delivery System=="<<endl;
    cout<<"====================================="<<endl;
}
void showmenu(){
    system("cls");
    
    cout << "\n=== Food Delivery System Menu ===" << endl;
    setcolor(14);
    cout << "1. Display Menu" << endl;
    cout << "2. Place Order" << endl; 
    cout << "3. Track Order" << endl;
    cout << "4. View Order History" << endl;
    cout << "5. Process Orders" << endl;
    cout << "6. Exit" << endl;
    setcolor(7);
    cout << "-------------------------------------" << endl;
}
void enterchoise(){
    setcolor(15);
    cout<<"Enter your choice: ";
}

void handleInvalidInput() {
    setcolor(12);
    cout << "Invalid input. Please try again." << endl;
    setcolor(7);
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int main() {
    showtitle();
    Restaurant restaurant;
    
    // Initialize menu items
    vector<MenuItem*> menuItems = {
        new MenuItem("CH1", "Kung Pao Chicken", 15.99, "Chinese Food", 25),
        new MenuItem("CH2", "Sweet & Sour Pork", 16.99, "Chinese Food", 25),
        new MenuItem("CH3", "Chow Mein", 13.99, "Chinese Food", 20),
        new MenuItem("CH4", "Spring Rolls", 7.99, "Chinese Food", 15),
        new MenuItem("CH5", "Fried Rice", 11.99, "Chinese Food", 20),
        new MenuItem("IT1", "Spaghetti Carbonara", 14.99, "Italian Food", 25),
        new MenuItem("IT2", "Fettuccine Alfredo", 15.99, "Italian Food", 25),
        new MenuItem("IT3", "Lasagna", 16.99, "Italian Food", 30),
        new MenuItem("IT4", "Risotto", 17.99, "Italian Food", 30),
        new MenuItem("IT5", "Tiramisu", 8.99, "Italian Dessert", 10),
        new MenuItem("MX1", "Beef Tacos", 12.99, "Mexican Food", 20),
        new MenuItem("MX2", "Chicken Quesadilla", 13.99, "Mexican Food", 20),
        new MenuItem("MX3", "Beef Burrito", 14.99, "Mexican Food", 25),
        new MenuItem("MX4", "Nachos Supreme", 11.99, "Mexican Food", 15),
        new MenuItem("MX5", "Guacamole & Chips", 8.99, "Mexican Food", 10),
        new MenuItem("SF1", "Grilled Salmon", 19.99, "Seafood", 25),
        new MenuItem("SF2", "Fish & Chips", 16.99, "Seafood", 20),
        new MenuItem("SF3", "Shrimp Scampi", 18.99, "Seafood", 25),
        new MenuItem("SF4", "Lobster Tail", 29.99, "Seafood", 35),
        new MenuItem("SF5", "Calamari", 13.99, "Seafood", 20),
        new MenuItem("DF1", "Chicken Biryani", 16.99, "Desi Food", 25),
        new MenuItem("DF2", "Beef Karahi", 18.99, "Desi Food", 30),
        new MenuItem("DF3", "Chicken Tikka", 14.99, "Desi Food", 20),
        new MenuItem("DF4", "Seekh Kabab", 12.99, "Desi Food", 15),
        new MenuItem("DF5", "Nihari", 17.99, "Desi Food", 35),
        new MenuItem("DF6", "Butter Naan", 2.99, "Desi Food", 10),
        new MenuItem("DFD1", "Gulab Jamun", 5.99, "Desi Food Dessert", 5),
        new MenuItem("DFD2", "Kheer", 6.99, "Desi Food Dessert", 15),
        new MenuItem("DFB1", "Lassi", 4.99, "Desi Food Beverage", 5),
        new MenuItem("DFB2", "Chai", 2.99, "Desi Food Beverage", 5),
        new MenuItem("P1", "Margherita Pizza", 12.99, "Pizza", 20),
        new MenuItem("P2", "Pepperoni Pizza", 14.99, "Pizza", 20),
        new MenuItem("P3", "BBQ Chicken Pizza", 15.99, "Pizza", 20),
        new MenuItem("P4", "Vegetarian Pizza", 13.99, "Pizza", 20),
        new MenuItem("B1", "Classic Burger", 9.99, "Burger", 15),
        new MenuItem("B2", "Cheese Burger", 11.99, "Burger", 15),
        new MenuItem("B3", "Veggie Burger", 10.99, "Burger", 15),
        new MenuItem("S1", "Caesar Salad", 8.99, "Salad", 10),
        new MenuItem("S2", "Greek Salad", 9.99, "Salad", 10),
        new MenuItem("D1", "Chocolate Cake", 6.99, "Dessert", 5),
        new MenuItem("D2", "Ice Cream", 4.99, "Dessert", 5),
        new MenuItem("D3", "Apple Pie", 5.99, "Dessert", 5),  
        new MenuItem("BV1", "Mango Juice", 6.99, "Beverage", 5),
        new MenuItem("BV2", "Lemonade", 4.99, "Beverage", 5),
        new MenuItem("BV3", "Water", 2.99, "Beverage", 5),
        new MenuItem("BV4", "Soda", 3.99, "Beverage", 5)
    };
    
    for (auto item : menuItems) {
        restaurant.addMenuItem(item);
    }
    
    int choice;
    while (true) {
        showmenu();
        enterchoise();
        
        if (!(cin >> choice)) {
            handleInvalidInput();
            continue;
        }
        
        switch (choice) {
            case 1:
                restaurant.displayMenu();
                cout << "\nPress any key to return to main menu...";
                _getch();
                break;
            case 2: {
                string customerName, phoneNumber;
                cout << "Enter customer name: ";
                cin.ignore();
                getline(cin, customerName);
                
                if (!restaurant.validateInput(customerName, "name")) {
                    setcolor(12);
                    cout << "Invalid name format. Please use only letters, spaces, and hyphens." << endl;
                    setcolor(7);
                    break;
                }
                
                do {
                    cout << "Enter phone number (format: XXX-XXX-XXXX): ";
                    getline(cin, phoneNumber);
                } while (!restaurant.validatePhoneNumber(phoneNumber));
                
                string street, city, zip;
                cout << "Enter delivery address:\n";
                cout << "Street address: ";
                getline(cin, street);
                cout << "City: ";
                getline(cin, city);
                cout << "ZIP code: ";
                getline(cin, zip);
                string cardNumber, expiry, cvv;
                do {
                    cout << "Enter credit card number (16 digits): ";
                    getline(cin, cardNumber);
                } while (!Payment::validateCardNumber(cardNumber));
                string cardType = Payment::detectCardType(cardNumber);
                cout << "Card Type: " << cardType << endl;
                cout << "Enter expiry date (MM/YY): ";
                getline(cin, expiry);
                cout << "Enter CVV: ";
                getline(cin, cvv);
                Address* address = new Address(street, city, zip);
                Payment* payment = new Payment(cardNumber, cardType, expiry, cvv);
                string newOrderId = "ORD" + to_string(rand() % 10000);
                Order* order = new Order(newOrderId, customerName, address, payment, phoneNumber);
                string specialInstructions;
                cout << "Special instructions (press Enter to skip): ";
                getline(cin, specialInstructions);
                if (!specialInstructions.empty()) {
                    order->setSpecialInstructions(specialInstructions);
                }
                while (true) {
                    restaurant.displayMenu();
                    string itemId;
                    cout << "\nEnter item ID (or 'done' to finish): ";
                    cin >> itemId;                 
                    if (itemId == "done") break;
                    // lookup item in menu
                    if(restaurant.menuMap.find(itemId) != restaurant.menuMap.end()) {
                        order->addItem(restaurant.menuMap[itemId]);
                    } else {
                        cout << "Invalid item ID, try again." << endl;
                    }
                }            
                restaurant.placeOrder(order);
                restaurant.displayOrderDetails(order);
                break;
            }
            case 3: {
                string orderId;
                setcolor(15);
                cout << "Enter order ID to track: ";
                setcolor(7);
                cin >> orderId;
                restaurant.trackOrder(orderId);
                break;
            }
            case 4:
                restaurant.viewOrderHistory();
                cout << "\nPress any key to return to main menu...";
                _getch();
                break;
            case 5:
                restaurant.processOrders();
                cout << "\nPress any key to return to main menu...";
                _getch();
                break;
            case 6:
                setcolor(5);
                cout << "Thank you for using the Food Ordering System!" << endl;
                setcolor(7);
                return 0;
            default:
                handleInvalidInput();
        }
    }
    return 0;
}

