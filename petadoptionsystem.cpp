#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <limits>
#include <cctype>
#include <regex>
#include <cstdlib>
#include <termios.h>
#include <unistd.h>
#include <stdexcept>
#include <memory>

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
    
    virtual void showDashboard() = 0;
    virtual void performAction(PetAdoptionSystem& system) = 0;
    
    bool authenticate(string uname, string pwd) const {
        return (username == uname && password == pwd);
    }
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
            users.push_back(make_unique<Admin>("admin", "admin123"));
            saveUsersToFile();
        }
        pets.push_back(Pet("Whiskers", "Siamese", 2, true));
        pets.push_back(Pet("Rex", "Labrador", 3, true));
    }
    
    // File handling functions
    void saveUsersToFile();
    void loadUsersFromFile();
    
    // Helper functions
    void clearScreen() const {
        system("cls || clear");
    }
    
    string getHiddenInput(const string& prompt) const {
        string input;
        cout << prompt;
        #ifdef _WIN32
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
        system("stty -echo");
        getline(cin, input);
        system("stty echo");
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
    void registerUser();
    User* login();
    
    // Pet operations
    void addPet(const string& name, const string& breed, int age, bool vaccinated) {
        pets.emplace_back(name, breed, age, vaccinated);
    }
    
    void editPet(size_t index, const string& name, const string& breed, int age, bool vaccinated) {
        if (index >= pets.size()) {
            throw out_of_range("Invalid pet index");
        }
        pets[index].setName(name);
        pets[index].setBreed(breed);
        pets[index].setAge(age);
        pets[index].setVaccinated(vaccinated);
    }
    
    void deletePet(size_t index) {
        if (index >= pets.size()) {
            throw out_of_range("Invalid pet index");
        }
        pets.erase(pets.begin() + index);
    }
    
    const vector<Pet>& getAllPets() const { return pets; }
    
    // Application operations
    void createApplication(const string& username, const string& petName) {
        applications.emplace_back(nextAppID++, username, petName);
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
                    break;
                }
            }
        } else {
            applications[index].reject();
        }
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
        users[index]->username = username;
        users[index]->password = password;
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
}

void PetAdoptionSystem::loadUsersFromFile() {
    ifstream inFile("users.dat");
    if (!inFile.is_open()) {
        return; // File doesn't exist yet
    }
    
    string line;
    while (getline(inFile, line)) {
        size_t pos1 = line.find(',');
        size_t pos2 = line.find(',', pos1+1);
        if (pos1 == string::npos || pos2 == string::npos) continue;
        
        string username = line.substr(0, pos1);
        string password = line.substr(pos1+1, pos2-pos1-1);
        Role role = static_cast<Role>(stoi(line.substr(pos2+1)));
        
        if (role == Role::ADMIN) {
            users.push_back(make_unique<Admin>(username, password));
        } else {
            users.push_back(make_unique<RegularUser>(username, password));
        }
    }
    inFile.close();
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
                    
                    system.addUser(make_unique<Admin>(username, password));
                    cout << "Admin added successfully!\n";
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
                            cout << "Username updated!\n";
                            break;
                        }
                        case 2: {
                            string newPwd = system.getHiddenInput("New password: ");
                            system.updateUser(idx, allUsers[idx]->getUsername(), newPwd);
                            cout << "Password updated!\n";
                            break;
                        }
                        case 3: {
                            system.deleteUser(idx);
                            cout << "User deleted!\n";
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
                    cout << "1. By Name\n2. By Breed\n3. By Age Range\n0. Back\n";
                    int searchChoice = system.getNumericInput("Enter choice: ", 0, 3);
                    
                    if (searchChoice == 0) break;
                    
                    unique_ptr<SearchStrategy> strategy;
                    switch (searchChoice) {
                        case 1: {
                            string name = system.getValidatedInput(
                                "Enter pet name to search: ", isValidName, "Invalid name");
                            strategy = make_unique<NameSearchStrategy>(name);
                            break;
                        }
                        case 2: {
                            string breed = system.getValidatedInput(
                                "Enter breed to search: ", isValidBreed, "Invalid breed");
                            strategy = make_unique<BreedSearchStrategy>(breed);
                            break;
                        }
                        case 3: {
                            int minAge = system.getNumericInput("Enter minimum age: ", 0, 30);
                            int maxAge = system.getNumericInput("Enter maximum age: ", minAge, 30);
                            strategy = make_unique<AgeRangeSearchStrategy>(minAge, maxAge);
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
                    if (availableIndices.empty()) {
                        cout << "No pets available for adoption.\n";
                        break;
                    }
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
        cout << "1. Login\n";
        cout << "2. Register\n";
        cout << "3. Exit\n";
        
        try {
            choice = getNumericInput("Enter choice: ", 1, 3);
            
            switch (choice) {
                case 1: { // Login
                    User* user = login();
                    if (user) {
                        user->performAction(*this);
                    }
                    break;
                }
                case 2: // Register
                    registerUser();
                    break;
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

void PetAdoptionSystem::registerUser() {
    clearScreen();
    cout << "\n=== REGISTRATION ===\n";
    cout << "Note: Only regular user registration is allowed\n";
    cout << "Admin accounts must be created by system administrators\n\n";
    
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
        
        addUser(make_unique<RegularUser>(username, password));
        cout << "Registration successful!\n";
    } catch (const InvalidInputException& e) {
        cout << "Registration failed: " << e.what() << "\n";
        cout << "1. Try again\n0. Back to menu\n";
        int retry = getNumericInput("Enter choice: ", 0, 1);
        if (retry == 1) registerUser();
    }
}

User* PetAdoptionSystem::login() {
    clearScreen();
    cout << "\n=== LOGIN ===\n";
    cout << "Select role:\n";
    cout << "1. Admin\n";
    cout << "2. User\n";
    cout << "0. Back to main menu\n";
    
    try {
        int roleChoice = getNumericInput("Enter choice: ", 0, 2);
        if (roleChoice == 0) return nullptr;
        
        Role selectedRole = (roleChoice == 1) ? Role::ADMIN : Role::USER;
        
        // Default admin shortcut
        if (selectedRole == Role::ADMIN) {
            for (const auto& user : users) {
                if (user->getRole() == Role::ADMIN &&
                    user->getUsername() == "admin" &&
                    user->getPassword() == "admin123") {
                    cout << "\nDefault admin login successful!\n";
                    return user.get();
                }
            }
            throw AuthenticationException("Default admin not found");
        }
        
        string username = getValidatedInput(
            "Username (or '0' to cancel): ",
            [](const string& s) { return isValidUsername(s) || s == "0"; },
            "Invalid username format");
        
        if (username == "0") return nullptr;
        
        string password = getHiddenInput("Password (or '0' to cancel): ");
        if (password == "0") return nullptr;
        
        for (const auto& user : users) {
            if (user->getRole() == selectedRole &&
                user->authenticate(username, password)) {
                cout << "\nLogin successful!\n";
                return user.get();
            }
        }
        
        throw AuthenticationException("Invalid credentials or role mismatch");
    } catch (const AuthenticationException& e) {
        cout << "Login failed: " << e.what() << "\n";
        cout << "1. Try again\n0. Back to menu\n";
        int retry = getNumericInput("Enter choice: ", 0, 1);
        return (retry == 1) ? login() : nullptr;
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
