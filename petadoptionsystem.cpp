#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <limits>
#include <cctype>
#include <regex>
#include <cstdlib>
#include <stdexcept>
#include <memory>
#include <iomanip>

// Cross-platform terminal handling
#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
#else
    #include <termios.h>
    #include <unistd.h>
#endif

using namespace std;

// User roles
enum class Role { ADMIN, USER };

// Forward declarations
class User;
class Admin;
class RegularUser;
class Pet;
class Application;
class PetAdoptionSystem;

// Custom exceptions
class InvalidInputException : public runtime_error {
public:
    InvalidInputException(const string& msg) : runtime_error(msg) {}
};

class FileOperationException : public runtime_error {
public:
    FileOperationException(const string& msg) : runtime_error(msg) {}
};

class AuthenticationException : public runtime_error {
public:
    AuthenticationException(const string& msg) : runtime_error(msg) {}
};

class AuthorizationException : public runtime_error {
public:
    AuthorizationException(const string& msg) : runtime_error(msg) {}
};

// Strategy Pattern: Search Strategy
class SearchStrategy {
public:
    virtual ~SearchStrategy() = default;
    virtual vector<Pet> search(const vector<Pet>& pets) = 0;
};

class NameSearchStrategy : public SearchStrategy {
private:
    string name;
public:
    NameSearchStrategy(const string& n) : name(n) {}
    vector<Pet> search(const vector<Pet>& pets) override;
};

class BreedSearchStrategy : public SearchStrategy {
private:
    string breed;
public:
    BreedSearchStrategy(const string& b) : breed(b) {}
    vector<Pet> search(const vector<Pet>& pets) override;
};

class AgeRangeSearchStrategy : public SearchStrategy {
private:
    int minAge;
    int maxAge;
public:
    AgeRangeSearchStrategy(int min, int max) : minAge(min), maxAge(max) {}
    vector<Pet> search(const vector<Pet>& pets) override;
};

// Pet class
class Pet {
private:
    string name;
    string breed;
    int age;
    bool vaccinated;
    bool adopted;
public:
    Pet(string n, string b, int a, bool v)
        : name(n), breed(b), age(a), vaccinated(v), adopted(false) {}
    
    // Serialization for file storage
    string serialize() const {
        return name + "," + breed + "," + to_string(age) + "," + 
               (vaccinated ? "1" : "0") + "," + (adopted ? "1" : "0");
    }
    
    // Static method to deserialize from string
    static Pet deserialize(const string& data) {
        size_t pos1 = data.find(',');
        size_t pos2 = data.find(',', pos1+1);
        size_t pos3 = data.find(',', pos2+1);
        size_t pos4 = data.find(',', pos3+1);
        
        if (pos1 == string::npos || pos2 == string::npos || 
            pos3 == string::npos || pos4 == string::npos) {
            throw InvalidInputException("Invalid pet data format");
        }
        
        string name = data.substr(0, pos1);
        string breed = data.substr(pos1+1, pos2-pos1-1);
        int age = stoi(data.substr(pos2+1, pos3-pos2-1));
        bool vaccinated = (data.substr(pos3+1, pos4-pos3-1) == "1");
        
        Pet pet(name, breed, age, vaccinated);
        if (data.substr(pos4+1) == "1") {
            pet.markAsAdopted();
        }
        return pet;
    }
    
    string getName() const { return name; }
    string getBreed() const { return breed; }
    int getAge() const { return age; }
    bool isVaccinated() const { return vaccinated; }
    bool isAdopted() const { return adopted; }
    void markAsAdopted() { adopted = true; }
    void setVaccinated(bool status) { vaccinated = status; }
    void setAge(int newAge) { age = newAge; }
    void setName(const string& newName) { name = newName; }
    void setBreed(const string& newBreed) { breed = newBreed; }
};

// Application class
class Application {
private:
    int id;
    string username;
    string petName;
    string status;
public:
    Application(int i, string uname, string pname)
        : id(i), username(uname), petName(pname), status("Pending") {}
    
    // Serialization for file storage
    string serialize() const {
        return to_string(id) + "," + username + "," + petName + "," + status;
    }
    
    // Static method to deserialize from string
    static Application deserialize(const string& data) {
        size_t pos1 = data.find(',');
        size_t pos2 = data.find(',', pos1+1);
        size_t pos3 = data.find(',', pos2+1);
        
        if (pos1 == string::npos || pos2 == string::npos || pos3 == string::npos) {
            throw InvalidInputException("Invalid application data format");
        }
        
        int id = stoi(data.substr(0, pos1));
        string username = data.substr(pos1+1, pos2-pos1-1);
        string petName = data.substr(pos2+1, pos3-pos2-1);
        string status = data.substr(pos3+1);
        
        Application app(id, username, petName);
        if (status == "Approved") {
            app.approve();
        } else if (status == "Rejected") {
            app.reject();
        }
        return app;
    }
    
    int getID() const { return id; }
    string getPetName() const { return petName; }
    string getUsername() const { return username; }
    string getStatus() const { return status; }
    void approve() { status = "Approved"; }
    void reject() { status = "Rejected"; }
};

// User class (Abstract)
class User {
protected:
    string username;
    string password;
    Role role;
public:
    User(string uname, string pwd, Role r) : username(uname), password(pwd), role(r) {}
    virtual ~User() = default;
    
    string getUsername() const { return username; }
    string getPassword() const { return password; }
    Role getRole() const { return role; }
    void setUsername(const string& uname) { username = uname; }
    void setPassword(const string& pwd) { password = pwd; }
    
    virtual void showDashboard() = 0;
    virtual void performAction(PetAdoptionSystem& system) = 0;
    
    bool authenticate(string uname, string pwd) const {
        return (username == uname && password == pwd);
    }
    
    friend class PetAdoptionSystem;
};

// Admin class
class Admin : public User {
public:
    Admin(string uname, string pwd) : User(uname, pwd, Role::ADMIN) {}
    
    void showDashboard() override {
        cout << "\n==== ADMIN DASHBOARD ====\n";
        cout << "1. Add Another Admin\n";
        cout << "2. Manage User Accounts\n";
        cout << "3. Manage Pet Records\n";
        cout << "4. Process Applications\n";
        cout << "5. Search Pets\n";
        cout << "6. Logout\n";
    }
    
    void performAction(PetAdoptionSystem& system) override;
};

// Regular User class
class RegularUser : public User {
public:
    RegularUser(string uname, string pwd) : User(uname, pwd, Role::USER) {}
    
    void showDashboard() override {
        cout << "\n==== USER DASHBOARD ====\n";
        cout << "1. Browse Pets\n";
        cout << "2. Check Application Status\n";
        cout << "3. View History\n";
        cout << "4. Logout\n";
    }
    
    void performAction(PetAdoptionSystem& system) override;
};

// Singleton Pattern: PetAdoptionSystem
class PetAdoptionSystem {
private:
    static PetAdoptionSystem* instance;
    vector<unique_ptr<User>> users;
    vector<Pet> pets;
    vector<Application> applications;
    int nextAppID = 1;
    
    // Private constructor for singleton
    PetAdoptionSystem() {
        loadUsersFromFile();
        if (users.empty()) {
            users.push_back(unique_ptr<User>(new Admin("admin", "admin123")));
            saveUsersToFile();
        }
        
        loadPetsFromFile();
        // Add default pets only if no pets were loaded
        if (pets.empty()) {
            pets.push_back(Pet("Whiskers", "Siamese", 2, true));
            pets.push_back(Pet("Rex", "Labrador", 3, true));
            savePetsToFile();
        }
        
        loadApplicationsFromFile();
    }
    
    // File handling functions
    void saveUsersToFile();
    void loadUsersFromFile();
    void savePetsToFile();
    void loadPetsFromFile();
    void saveApplicationsToFile();
    void loadApplicationsFromFile();
    
    // Helper functions
    void clearScreen() const {
        system("cls || clear");
    }
    
    string getHiddenInput(const string& prompt) const {
        string input;
        cout << prompt;
        
        #ifdef _WIN32
        // Windows implementation
        char ch;
        while ((ch = _getch()) != '\r') {
            if (ch == '\b') {
                if (!input.empty()) {
                    cout << "\b \b";
                    input.pop_back();
                }
            } else {
                cout << '*';
                input.push_back(ch);
            }
        }
        #else
        // Unix/Linux/macOS implementation using termios directly
        termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ECHO | ICANON);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        
        char ch;
        while ((ch = getchar()) != '\n') {
            if (ch == 127 || ch == 8) { // Backspace or Delete
                if (!input.empty()) {
                    cout << "\b \b";
                    input.pop_back();
                }
            } else {
                cout << '*';
                input.push_back(ch);
            }
        }
        
        // Restore terminal settings
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        #endif
        
        cout << endl;
        return input;
    }
    
    string getValidatedInput(const string& prompt, bool (*validator)(const string&), 
                           const string& errorMsg, int maxAttempts = 3) const {
        string input;
        int attempts = 0;
        while (attempts < maxAttempts) {
            cout << prompt;
            getline(cin >> ws, input);
            if (input == "0") return input;
            if (validator(input)) {
                return input;
            }
            cout << errorMsg << " (" << maxAttempts - attempts - 1
                 << " attempts remaining, or '0' to cancel)\n";
            attempts++;
        }
        throw InvalidInputException("Too many failed attempts");
    }
    
    int getNumericInput(const string& prompt, int min, int max, int maxAttempts = 3) const {
        int value;
        int attempts = 0;
        string input;
        
        while (attempts < maxAttempts) {
            cout << prompt;
            getline(cin, input);
            
            if (!regex_match(input, regex("^\\s*\\d+\\s*$"))) {
                cout << "Invalid input format. Please enter a whole number between " 
                     << min << " and " << max << ".\n";
                attempts++;
                continue;
            }
            
            try {
                value = stoi(input);
                if (value >= min && value <= max) {
                    return value;
                }
                cout << "Please enter between " << min << " and " << max
                     << " (" << maxAttempts - attempts - 1 << " attempts left)\n";
            } catch (...) {
                cout << "Invalid input. Please enter a number between "
                     << min << " and " << max << ".\n";
            }
            attempts++;
        }
        throw InvalidInputException("Too many failed attempts");
    }
    
    int getAgeInput(const string& prompt) const {
        string input;
        while (true) {
            cout << prompt;
            getline(cin, input);
            
            if (regex_match(input, regex("^\\d+$"))) {
                return stoi(input);
            }
            
            smatch matches;
            if (regex_match(input, matches, regex("^(\\d+)\\s*(years?|months?)$"))) {
                int value = stoi(matches[1].str());
                string unit = matches[2].str();
                if (unit.find("month") != string::npos) {
                    return value / 12;
                }
                return value;
            }
            cout << "Invalid age format. Please enter like '2', '3 years', or '6 months'\n";
        }
    }

public:
    // Delete copy constructor and assignment operator
    PetAdoptionSystem(const PetAdoptionSystem&) = delete;
    PetAdoptionSystem& operator=(const PetAdoptionSystem&) = delete;
    
    // Singleton access method
    static PetAdoptionSystem& getInstance() {
        if (!instance) {
            instance = new PetAdoptionSystem();
        }
        return *instance;
    }
    
    // Destructor
    ~PetAdoptionSystem() {
        saveUsersToFile();
    }
    
    // Main system operations
    void run();
    void registerUser(Role role);
    User* login(Role role);
    
    // Pet operations
    void addPet(const string& name, const string& breed, int age, bool vaccinated) {
        pets.emplace_back(name, breed, age, vaccinated);
        savePetsToFile(); // Save after adding
    }
    
    void editPet(size_t index, const string& name, const string& breed, int age, bool vaccinated) {
        if (index >= pets.size()) {
            throw out_of_range("Invalid pet index");
        }
        pets[index].setName(name);
        pets[index].setBreed(breed);
        pets[index].setAge(age);
        pets[index].setVaccinated(vaccinated);
        savePetsToFile(); // Save after editing
    }
    
    void deletePet(size_t index) {
        if (index >= pets.size()) {
            throw out_of_range("Invalid pet index");
        }
        pets.erase(pets.begin() + index);
        savePetsToFile(); // Save after deleting
    }
    
    void viewAllPets() const {
        clearScreen();
        cout << "\n=== ALL PET RECORDS ===\n";
        if (pets.empty()) {
            cout << "No pets in the system.\n";
            return;
        }
        
        cout << "ID  | Name          | Breed         | Age | Vaccinated | Status\n";
        cout << "----+---------------+---------------+-----+------------+--------\n";
        
        for (size_t i = 0; i < pets.size(); ++i) {
            const Pet& pet = pets[i];
            cout << left << setw(4) << i+1 << "| "
                 << setw(15) << pet.getName() << "| "
                 << setw(15) << pet.getBreed() << "| "
                 << setw(5) << pet.getAge() << "| "
                 << setw(12) << (pet.isVaccinated() ? "Yes" : "No") << "| "
                 << (pet.isAdopted() ? "Adopted" : "Available") << "\n";
        }
    }
    
    const vector<Pet>& getAllPets() const { return pets; }
    
    // Application operations
    void createApplication(const string& username, const string& petName) {
        applications.emplace_back(nextAppID++, username, petName);
        saveApplicationsToFile(); // Save when a new application is created
    }
    
    void processApplication(size_t index, bool approve) {
        if (index >= applications.size()) {
            throw out_of_range("Invalid application index");
        }
        
        if (approve) {
            applications[index].approve();
            for (auto& pet : pets) {
                if (pet.getName() == applications[index].getPetName()) {
                    pet.markAsAdopted();
                    savePetsToFile(); // Save pet status change
                    break;
                }
            }
        } else {
            applications[index].reject();
        }
        
        // Save applications to file
        saveApplicationsToFile();
    }
    
    const vector<Application>& getAllApplications() const { return applications; }
    
    // Search operations
    vector<Pet> searchPets(unique_ptr<SearchStrategy> strategy) const {
        return strategy->search(pets);
    }
    
    // User management
    void addUser(unique_ptr<User> user) {
        users.push_back(move(user));
        saveUsersToFile();
    }
    
    void deleteUser(size_t index) {
        if (index >= users.size()) {
            throw out_of_range("Invalid user index");
        }
        users.erase(users.begin() + index);
        saveUsersToFile();
    }
    
    void updateUser(size_t index, const string& username, const string& password) {
        if (index >= users.size()) {
            throw out_of_range("Invalid user index");
        }
        users[index]->setUsername(username);
        users[index]->setPassword(password);
        saveUsersToFile();
    }
    
    const vector<unique_ptr<User>>& getAllUsers() const { return users; }
    
    // Friend classes for protected access
    friend class Admin;
    friend class RegularUser;
};

// Initialize singleton instance
PetAdoptionSystem* PetAdoptionSystem::instance = nullptr;

// Validation functions
bool isValidUsername(const string& username) {
    if (username.length() < 4 || username.length() > 20) return false;
    if (!regex_match(username, regex("^[A-Za-z0-9 ]+$"))) return false;
    for (size_t i = 1; i < username.size(); ++i)
        if (isspace(username[i]) && isspace(username[i-1])) return false;
    return true;
}

bool isValidPassword(const string& password) {
    return !password.empty();
}

bool isValidName(const string& name) {
    if (name.empty()) return false;
    if (!regex_match(name, regex("^[A-Za-z0-9 ]+$"))) return false;
    for (size_t i = 1; i < name.size(); ++i)
        if (isspace(name[i]) && isspace(name[i-1])) return false;
    return true;
}

bool isValidBreed(const string& breed) {
    return isValidName(breed);
}

// SearchStrategy implementations
vector<Pet> NameSearchStrategy::search(const vector<Pet>& pets) {
    vector<Pet> results;
    for (const auto& pet : pets) {
        if (pet.getName().find(name) != string::npos) {
            results.push_back(pet);
        }
    }
    return results;
}

vector<Pet> BreedSearchStrategy::search(const vector<Pet>& pets) {
    vector<Pet> results;
    for (const auto& pet : pets) {
        if (pet.getBreed().find(breed) != string::npos) {
            results.push_back(pet);
        }
    }
    return results;
}

vector<Pet> AgeRangeSearchStrategy::search(const vector<Pet>& pets) {
    vector<Pet> results;
    for (const auto& pet : pets) {
        if (pet.getAge() >= minAge && pet.getAge() <= maxAge) {
            results.push_back(pet);
        }
    }
    return results;
}

// File handling implementations
void PetAdoptionSystem::saveUsersToFile() {
    ofstream outFile("users.dat");
    if (!outFile.is_open()) {
        throw FileOperationException("Failed to open users file for writing");
    }
    
    for (const auto& user : users) {
        outFile << user->getUsername() << ","
                << user->getPassword() << ","
                << static_cast<int>(user->getRole()) << "\n";
    }
    outFile.close();
    cout << "User credentials saved successfully.\n";
}

void PetAdoptionSystem::savePetsToFile() {
    ofstream outFile("pets.dat");
    if (!outFile.is_open()) {
        throw FileOperationException("Failed to open pets file for writing");
    }
    
    for (const auto& pet : pets) {
        outFile << pet.serialize() << "\n";
    }
    outFile.close();
    cout << "Pets saved successfully.\n";
}

void PetAdoptionSystem::loadUsersFromFile() {
    ifstream inFile("users.dat");
    if (!inFile.is_open()) {
        return; // File doesn't exist yet
    }
    
    string line;
    int userCount = 0;
    while (getline(inFile, line)) {
        size_t pos1 = line.find(',');
        size_t pos2 = line.find(',', pos1+1);
        if (pos1 == string::npos || pos2 == string::npos) continue;
        
        string username = line.substr(0, pos1);
        string password = line.substr(pos1+1, pos2-pos1-1);
        Role role = static_cast<Role>(stoi(line.substr(pos2+1)));
        
        if (role == Role::ADMIN) {
            users.push_back(unique_ptr<User>(new Admin(username, password)));
        } else {
            users.push_back(unique_ptr<User>(new RegularUser(username, password)));
        }
        userCount++;
    }
    inFile.close();
    cout << userCount << " user(s) loaded from database.\n";
}

void PetAdoptionSystem::loadPetsFromFile() {
    ifstream inFile("pets.dat");
    if (!inFile.is_open()) {
        return; // File doesn't exist yet
    }
    
    string line;
    while (getline(inFile, line)) {
        try {
            Pet pet = Pet::deserialize(line);
            pets.push_back(pet);
        } catch (const exception& e) {
            cerr << "Error loading pet: " << e.what() << "\n";
            continue; // Skip invalid entries
        }
    }
    inFile.close();
    cout << pets.size() << " pets loaded from file.\n";
}

void PetAdoptionSystem::saveApplicationsToFile() {
    ofstream outFile("applications.dat");
    if (!outFile.is_open()) {
        throw FileOperationException("Failed to open applications file for writing");
    }
    
    // Also save the next application ID
    outFile << "NEXT_ID:" << nextAppID << "\n";
    
    for (const auto& app : applications) {
        outFile << app.serialize() << "\n";
    }
    outFile.close();
    cout << "Applications saved successfully.\n";
}

void PetAdoptionSystem::loadApplicationsFromFile() {
    ifstream inFile("applications.dat");
    if (!inFile.is_open()) {
        return; // File doesn't exist yet
    }
    
    applications.clear();
    string line;
    
    // First line should be the next ID
    if (getline(inFile, line) && line.substr(0, 8) == "NEXT_ID:") {
        nextAppID = stoi(line.substr(8));
    } else {
        // If not found, reset the file pointer to the beginning
        inFile.clear();
        inFile.seekg(0, ios::beg);
    }
    
    // Read all applications
    while (getline(inFile, line)) {
        try {
            Application app = Application::deserialize(line);
            applications.push_back(app);
        } catch (const exception& e) {
            cerr << "Error loading application: " << e.what() << "\n";
            continue; // Skip invalid entries
        }
    }
    inFile.close();
    cout << applications.size() << " applications loaded from file.\n";
}

// Admin actions implementation
void Admin::performAction(PetAdoptionSystem& system) {
    int choice;
    do {
        system.clearScreen();
        showDashboard();
        
        try {
            choice = system.getNumericInput("Enter choice: ", 1, 6);
            
            switch (choice) {
                case 1: { // Add Admin
                    system.clearScreen();
                    cout << "\n=== ADD NEW ADMIN ===\n";
                    string username = system.getValidatedInput(
                        "Admin username (4-20 chars, case-sensitive): ", 
                        isValidUsername, "Invalid username format");
                    
                    for (const auto& user : system.getAllUsers()) {
                        if (user->getUsername() == username) {
                            throw InvalidInputException("Username already exists");
                        }
                    }
                    
                    string password = system.getHiddenInput("Password: ");
                    if (!isValidPassword(password)) {
                        throw InvalidInputException("Invalid password");
                    }
                    
                    system.addUser(unique_ptr<User>(new Admin(username, password)));
                    // The addUser method already calls saveUsersToFile() internally
                    cout << "Admin added successfully! Credentials have been saved.\n";
                    break;
                }
                case 2: { // Manage Users
                    system.clearScreen();
                    cout << "\n=== MANAGE USER ACCOUNTS ===\n";
                    
                    const auto& allUsers = system.getAllUsers();
                    if (allUsers.empty()) {
                        cout << "No users found.\n";
                        break;
                    }
                    
                    for (size_t i = 0; i < allUsers.size(); ++i) {
                        cout << i+1 << ". " << allUsers[i]->getUsername() 
                             << " (" << (allUsers[i]->getRole() == Role::ADMIN ? "Admin" : "User") << ")\n";
                    }
                    
                    int idx = system.getNumericInput("Select user (0 to cancel): ", 0, allUsers.size()) - 1;
                    if (idx == -1) break;
                    
                    cout << "1. Edit Username\n2. Edit Password\n3. Delete User\n0. Back\n";
                    int action = system.getNumericInput("Enter action: ", 0, 3);
                    if (action == 0) break;
                    
                    switch (action) {
                        case 1: {
                            string newName = system.getValidatedInput(
                                "New username: ", isValidUsername, "Invalid username");
                            system.updateUser(idx, newName, allUsers[idx]->getPassword());
                            // The updateUser method already calls saveUsersToFile() internally
                            cout << "Username updated and saved!\n";
                            break;
                        }
                        case 2: {
                            string newPwd = system.getHiddenInput("New password: ");
                            system.updateUser(idx, allUsers[idx]->getUsername(), newPwd);
                            // The updateUser method already calls saveUsersToFile() internally
                            cout << "Password updated and saved!\n";
                            break;
                        }
                        case 3: {
                            system.deleteUser(idx);
                            // The deleteUser method already calls saveUsersToFile() internally
                            cout << "User deleted and database updated!\n";
                            break;
                        }
                    }
                    break;
                }
                case 3: { // Manage Pets
                    system.clearScreen();
                    cout << "\n=== MANAGE PETS ===\n";
                    cout << "1. Add Pet\n2. Edit Pet\n3. Delete Pet\n4. View All Pets\n0. Back\n";
                    int petChoice = system.getNumericInput("Enter choice: ", 0, 4);
                    
                    if (petChoice == 0) break;
                    
                    const auto& allPets = system.getAllPets();
                    
                    switch (petChoice) {
                        case 1: { // Add Pet
                            system.clearScreen();
                            cout << "\n=== ADD NEW PET ===\n";
                            string name = system.getValidatedInput(
                                "Pet name: ", isValidName, "Invalid name");
                            string breed = system.getValidatedInput(
                                "Breed: ", isValidBreed, "Invalid breed");
                            int age = system.getAgeInput("Age: ");
                            bool vaccinated = system.getNumericInput(
                                "Vaccinated? (1=Yes, 0=No): ", 0, 1);
                            
                            system.addPet(name, breed, age, vaccinated);
                            cout << "Pet added successfully!\n";
                            break;
                        }
                        case 2: { // Edit Pet
                            if (allPets.empty()) {
                                cout << "No pets available to edit.\n";
                                break;
                            }
                            
                            for (size_t i = 0; i < allPets.size(); ++i) {
                                cout << i+1 << ". " << allPets[i].getName() 
                                     << " (" << allPets[i].getBreed() << ")\n";
                            }
                            
                            int petIdx = system.getNumericInput(
                                "Select pet to edit (0 to cancel): ", 0, allPets.size()) - 1;
                            if (petIdx == -1) break;
                            
                            const Pet& pet = allPets[petIdx];
                            cout << "1. Name: " << pet.getName() << "\n";
                            cout << "2. Breed: " << pet.getBreed() << "\n";
                            cout << "3. Age: " << pet.getAge() << "\n";
                            cout << "4. Vaccinated: " << (pet.isVaccinated() ? "Yes" : "No") << "\n";
                            cout << "0. Back\n";
                            
                            int field = system.getNumericInput("Select field to edit: ", 0, 4);
                            if (field == 0) break;
                            
                            string newName = pet.getName();
                            string newBreed = pet.getBreed();
                            int newAge = pet.getAge();
                            bool newVax = pet.isVaccinated();
                            
                            switch (field) {
                                case 1:
                                    newName = system.getValidatedInput(
                                        "New name: ", isValidName, "Invalid name");
                                    break;
                                case 2:
                                    newBreed = system.getValidatedInput(
                                        "New breed: ", isValidBreed, "Invalid breed");
                                    break;
                                case 3:
                                    newAge = system.getAgeInput("New age: ");
                                    break;
                                case 4:
                                    newVax = system.getNumericInput(
                                        "Vaccinated? (1=Yes, 0=No): ", 0, 1);
                                    break;
                            }
                            
                            system.editPet(petIdx, newName, newBreed, newAge, newVax);
                            cout << "Pet updated successfully!\n";
                            break;
                        }
                        case 3: { // Delete Pet
                            if (allPets.empty()) {
                                cout << "No pets available to delete.\n";
                                break;
                            }
                            
                            for (size_t i = 0; i < allPets.size(); ++i) {
                                cout << i+1 << ". " << allPets[i].getName() 
                                     << " (" << allPets[i].getBreed() << ")\n";
                            }
                            
                            int petIdx = system.getNumericInput(
                                "Select pet to delete (0 to cancel): ", 0, allPets.size()) - 1;
                            if (petIdx == -1) break;
                            
                            system.deletePet(petIdx);
                            cout << "Pet deleted successfully!\n";
                            break;
                        }
                        case 4: { // View All Pets
                            system.clearScreen();
                            cout << "\n=== ALL PETS ===\n";
                            if (allPets.empty()) {
                                cout << "No pets in the system.\n";
                                break;
                            }
                            
                            for (size_t i = 0; i < allPets.size(); ++i) {
                                cout << i+1 << ". " << allPets[i].getName() 
                                     << " (" << allPets[i].getBreed() 
                                     << "), Age: " << allPets[i].getAge() 
                                     << ", Vaccinated: " << (allPets[i].isVaccinated() ? "Yes" : "No")
                                     << ", Status: " << (allPets[i].isAdopted() ? "Adopted" : "Available") << "\n";
                            }
                            break;
                        }
                    }
                    break;
                }
                case 4: { // Process Applications
                    system.clearScreen();
                    cout << "\n=== PROCESS APPLICATIONS ===\n";
                    
                    const auto& allApps = system.getAllApplications();
                    if (allApps.empty()) {
                        cout << "No applications to process.\n";
                        break;
                    }
                    
                    vector<size_t> pendingIndices;
                    for (size_t i = 0; i < allApps.size(); ++i) {
                        if (allApps[i].getStatus() == "Pending") {
                            cout << i+1 << ". ID: " << allApps[i].getID() 
                                 << ", User: " << allApps[i].getUsername() 
                                 << ", Pet: " << allApps[i].getPetName() << "\n";
                            pendingIndices.push_back(i);
                        }
                    }
                    
                    if (pendingIndices.empty()) {
                        cout << "No pending applications.\n";
                        break;
                    }
                    
                    int choice = system.getNumericInput(
                        "Select application to process (0 to cancel): ", 0, pendingIndices.size());
                    if (choice == 0) break;
                    
                    size_t appIdx = pendingIndices[choice-1];
                    cout << "1. Approve\n2. Reject\n0. Back\n";
                    int action = system.getNumericInput("Enter action: ", 0, 2);
                    
                    if (action == 1) {
                        system.processApplication(appIdx, true);
                        cout << "Application approved!\n";
                    } else if (action == 2) {
                        system.processApplication(appIdx, false);
                        cout << "Application rejected.\n";
                    }
                    break;
                }
                case 5: { // Search Pets
                    system.clearScreen();
                    cout << "\n=== SEARCH PETS ===\n";
                    cout << "1. By Name\n2. By Breed\n3. By Age Range\n4. View All Pets\n0. Back\n";
                    int searchChoice = system.getNumericInput("Enter choice: ", 0, 4);
                    
                    if (searchChoice == 0) break;
                    
                    if (searchChoice == 4) {
                        system.viewAllPets();
                        break;
                    }
                    
                    unique_ptr<SearchStrategy> strategy;
                    switch (searchChoice) {
                        case 1: {
                            string name = system.getValidatedInput(
                                "Enter pet name to search: ", isValidName, "Invalid name");
                            strategy.reset(new NameSearchStrategy(name));
                            break;
                        }
                        case 2: {
                            string breed = system.getValidatedInput(
                                "Enter breed to search: ", isValidBreed, "Invalid breed");
                            strategy.reset(new BreedSearchStrategy(breed));
                            break;
                        }
                        case 3: {
                            int minAge = system.getNumericInput("Enter minimum age: ", 0, 30);
                            int maxAge = system.getNumericInput("Enter maximum age: ", minAge, 30);
                            strategy.reset(new AgeRangeSearchStrategy(minAge, maxAge));
                            break;
                        }
                    }
                    
                    vector<Pet> results = system.searchPets(move(strategy));
                    if (results.empty()) {
                        cout << "No matching pets found.\n";
                    } else {
                        cout << "\n=== SEARCH RESULTS ===\n";
                        for (size_t i = 0; i < results.size(); ++i) {
                            cout << i+1 << ". " << results[i].getName() 
                                 << " (" << results[i].getBreed() 
                                 << "), Age: " << results[i].getAge() 
                                 << ", Vaccinated: " << (results[i].isVaccinated() ? "Yes" : "No")
                                 << ", Status: " << (results[i].isAdopted() ? "Adopted" : "Available") << "\n";
                        }
                    }
                    break;
                }
                case 6: // Logout
                    cout << "Logging out...\n";
                    break;
            }
        } catch (const InvalidInputException& e) {
            cout << "Error: " << e.what() << "\n";
        } catch (const exception& e) {
            cout << "An error occurred: " << e.what() << "\n";
        }
        
        if (choice != 6) { 
            cout << "\nPress Enter to continue..."; 
            cin.ignore(); 
            cin.get(); 
        }
    } while (choice != 6);
}

// RegularUser actions implementation
void RegularUser::performAction(PetAdoptionSystem& system) {
    int choice;
    do {
        system.clearScreen();
        showDashboard();
        
        try {
            choice = system.getNumericInput("Enter choice: ", 1, 4);
            
            switch (choice) {
                case 1: { // Browse Pets
                    system.clearScreen();
                    cout << "\n=== AVAILABLE PETS ===\n";
                    
                    const auto& allPets = system.getAllPets();
                    if (allPets.empty()) {
                        cout << "No pets available for adoption.\n";
                        break;
                    }
                    
                    vector<size_t> availableIndices;
                    for (size_t i = 0; i < allPets.size(); ++i) {
                        if (!allPets[i].isAdopted()) {
                            cout << i+1 << ". " << allPets[i].getName() 
                                 << " (" << allPets[i].getBreed() 
                                 << "), Age: " << allPets[i].getAge() 
                                 << ", Vaccinated: " << (allPets[i].isVaccinated() ? "Yes" : "No") << "\n";
                            availableIndices.push_back(i);
                        }
                    }
                    
                    cout << "\n0. Back\n";
                    int petChoice = system.getNumericInput(
                        "Select pet to apply for adoption (0 to cancel): ", 0, availableIndices.size());
                    if (petChoice == 0) break;
                    
                    size_t petIdx = availableIndices[petChoice-1];
                    system.createApplication(username, allPets[petIdx].getName());
                    cout << "Application submitted for " << allPets[petIdx].getName() << "!\n";
                    break;
                }
                case 2: { // Check Status
                    system.clearScreen();
                    cout << "\n=== APPLICATION STATUS ===\n";
                    
                    const auto& allApps = system.getAllApplications();
                    bool found = false;
                    
                    for (const auto& app : allApps) {
                        if (app.getUsername() == username) {
                            cout << "ID: " << app.getID() << ", Pet: " << app.getPetName() 
                                 << ", Status: " << app.getStatus() << "\n";
                            found = true;
                        }
                    }
                    
                    if (!found) {
                        cout << "No applications found.\n";
                    }
                    break;
                }
                case 3: { // View History
                    system.clearScreen();
                    cout << "\n=== ADOPTION HISTORY ===\n";
                    
                    const auto& allPets = system.getAllPets();
                    bool found = false;
                    
                    for (const auto& pet : allPets) {
                        if (pet.isAdopted()) {
                            cout << pet.getName() << " (" << pet.getBreed() << ")\n";
                            found = true;
                        }
                    }
                    
                    if (!found) {
                        cout << "No adoption history found.\n";
                    }
                    break;
                }
                case 4: // Logout
                    cout << "Logging out...\n";
                    break;
            }
        } catch (const exception& e) {
            cout << "An error occurred: " << e.what() << "\n";
        }
        
        if (choice != 4) { 
            cout << "\nPress Enter to continue..."; 
            cin.ignore(); 
            cin.get(); 
        }
    } while (choice != 4);
}

// Main system operations
void PetAdoptionSystem::run() {
    int choice;
    do {
        clearScreen();
        cout << "\n=== PET ADOPTION SYSTEM ===\n";
        cout << "1. Admin Access\n";
        cout << "2. User Access\n";
        cout << "3. Exit\n";
        
        try {
            choice = getNumericInput("Enter choice: ", 1, 3);
            
            switch (choice) {
                case 1: { // Admin Access
                    clearScreen();
                    cout << "\n=== ADMIN ACCESS ===\n";
                    cout << "1. Login\n";
                    cout << "0. Back\n";
                    
                    int adminChoice = getNumericInput("Enter choice: ", 0, 1);
                    if (adminChoice == 0) break;
                    
                    User* admin = login(Role::ADMIN);
                    if (admin) {
                        admin->performAction(*this);
                    }
                    break;
                }
                case 2: { // User Access
                    clearScreen();
                    cout << "\n=== USER ACCESS ===\n";
                    cout << "1. Login\n";
                    cout << "2. Register\n";
                    cout << "0. Back\n";
                    
                    int userChoice = getNumericInput("Enter choice: ", 0, 2);
                    if (userChoice == 0) break;
                    
                    if (userChoice == 1) {
                        User* user = login(Role::USER);
                        if (user) {
                            user->performAction(*this);
                        }
                    } else {
                        registerUser(Role::USER);
                    }
                    break;
                }
                case 3: // Exit
                    cout << "Exiting system...\n";
                    break;
            }
        } catch (const exception& e) {
            cout << "An error occurred: " << e.what() << "\n";
        }
        
        if (choice != 3) { 
            cout << "\nPress Enter to continue..."; 
            cin.ignore(); 
            cin.get(); 
        }
    } while (choice != 3);
}

void PetAdoptionSystem::registerUser(Role role) {
    if (role != Role::USER) {
        cout << "Only regular users can register. Admin accounts must be created by existing admins.\n";
        return;
    }
    
    clearScreen();
    cout << "\n=== USER REGISTRATION ===\n";
    
    try {
        string username = getValidatedInput(
            "Enter username (4-20 alphanumeric chars, '0' to cancel): ",
            isValidUsername,
            "Invalid username format");
        
        if (username == "0") return;
        
        for (const auto& user : users) {
            if (user->getUsername() == username) {
                throw InvalidInputException("Username already exists");
            }
        }
        
        string password = getHiddenInput("Enter password: ");
        if (!isValidPassword(password)) {
            throw InvalidInputException("Invalid password");
        }
        
        addUser(unique_ptr<User>(new RegularUser(username, password)));
        // The addUser method already calls saveUsersToFile() internally
        cout << "Registration successful! Your credentials have been saved.\n";
    } catch (const InvalidInputException& e) {
        cout << "Registration failed: " << e.what() << "\n";
        cout << "1. Try again\n0. Back to menu\n";
        int retry = getNumericInput("Enter choice: ", 0, 1);
        if (retry == 1) registerUser(role);
    }
}

User* PetAdoptionSystem::login(Role role) {
    clearScreen();
    cout << "\n=== " << (role == Role::ADMIN ? "ADMIN" : "USER") << " LOGIN ===\n";
    
    try {
        string username;
        string password;
        
        // Handle credential input
        if (role == Role::ADMIN) {
            username = getValidatedInput(
                "Admin username (or '0' to cancel): ",
                [](const string& s) { return isValidUsername(s) || s == "0"; },
                "Invalid username format");
        } else {
            username = getValidatedInput(
                "Username (or '0' to cancel): ",
                [](const string& s) { return isValidUsername(s) || s == "0"; },
                "Invalid username format");
        }
        
        if (username == "0") return nullptr;
        password = getHiddenInput("Password (or '0' to cancel): ");
        if (password == "0") return nullptr;
        
        // Special case for default admin
        if (role == Role::ADMIN && username == "admin" && password == "admin123") {
            for (const auto& user : users) {
                if (user->getUsername() == "admin") {
                    cout << "\nAdmin login successful!\n";
                    return user.get();
                }
            }
            
            // Create default admin if not found
            users.push_back(unique_ptr<User>(new Admin("admin", "admin123")));
            saveUsersToFile();
            cout << "Login credentials saved successfully.\n";
            cout << "\nDefault admin created and login successful!\n";
            return users.back().get();
        }
        
        // Check credentials against user database
        for (const auto& user : users) {
            if (user->getRole() == role && user->authenticate(username, password)) {
                cout << "\nLogin successful!\n";
                return user.get();
            }
        }
        
        throw AuthenticationException("Invalid credentials");
    } catch (const AuthenticationException& e) {
        cout << "Login failed: " << e.what() << "\n";
        cout << "1. Try again\n0. Back to menu\n";
        int retry = getNumericInput("Enter choice: ", 0, 1);
        return (retry == 1) ? login(role) : nullptr;
    } catch (const exception& e) {
        cout << "An error occurred during login: " << e.what() << "\n";
        return nullptr;
    }
}

int main() {
    try {
        PetAdoptionSystem& system = PetAdoptionSystem::getInstance();
        system.run();
    } catch (const exception& e) {
        cerr << "Fatal error: " << e.what() << endl;
        return 1;
    }
    return 0;
}
