#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <limits>
#include <cctype>
#include <regex>
#include <conio.h>
#include <stdlib.h>

using namespace std;

// User roles
enum class Role { ADMIN, USER };

// Forward declarations
class User;
class Admin;
class RegularUser;

// File handling functions
void saveUsersToFile(const vector<User*>& users);
vector<User*> loadUsersFromFile();

// Validation functions
bool isValidUsername(const string& username);
bool isValidPassword(const string& password);
bool isValidName(const string& name);
bool isValidBreed(const string& breed);
int getAgeInput(const string& prompt);

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

// User class
class User {
protected:
    string username;
    string password;
    Role role;
public:
    User(string uname, string pwd, Role r) : username(uname), password(pwd), role(r) {}
    string getUsername() const { return username; }
    string getPassword() const { return password; }
    Role getRole() const { return role; }
    virtual void showDashboard() = 0;
    bool authenticate(string uname, string pwd) {
        return (username == uname && password == pwd);
    }
    virtual ~User() {}
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
};

// Validation functions implementation
bool isValidUsername(const string& username) {
    // 4-20 chars, letters, numbers, and spaces allowed
    if (username.length() < 4 || username.length() > 20) return false;
    if (!regex_match(username, regex("^[A-Za-z0-9 ]+$"))) return false;
    // No consecutive spaces
    for (size_t i = 1; i < username.size(); ++i)
        if (isspace(username[i]) && isspace(username[i-1])) return false;
    return true;
}

bool isValidPassword(const string& password) {
    // No restrictions on password
    return !password.empty();
}

bool isValidName(const string& name) {
    if (name.empty()) return false;
    // Allow letters, numbers, and spaces
    if (!regex_match(name, regex("^[A-Za-z0-9 ]+$"))) return false;
    // No consecutive spaces
    for (size_t i = 1; i < name.size(); ++i)
        if (isspace(name[i]) && isspace(name[i-1])) return false;
    return true;
}

bool isValidBreed(const string& breed) {
    return isValidName(breed);
}

int getAgeInput(const string& prompt) {
    string input;
    while (true) {
        cout << prompt;
        getline(cin, input);
        
        // Check for simple numeric input
        if (regex_match(input, regex("^\\d+$"))) {
            return stoi(input);
        }
        // Check for "X years" or "X months" format
        smatch matches;
        if (regex_match(input, matches, regex("^(\\d+)\\s*(years?|months?)$"))) {
            int value = stoi(matches[1].str());
            string unit = matches[2].str();
            if (unit.find("month") != string::npos) {
                return value / 12; // Convert months to years
            }
            return value;
        }
        cout << "Invalid age format. Please enter like '2', '3 years', or '6 months'\n";
    }
}

// File handling functions implementation
void saveUsersToFile(const vector<User*>& users) {
    ofstream outFile("users.dat");
    if (outFile.is_open()) {
        for (const auto& user : users) {
            outFile << user->getUsername() << ","
                    << user->getPassword() << ","
                    << static_cast<int>(user->getRole()) << "\n";
        }
        outFile.close();
    }
}

vector<User*> loadUsersFromFile() {
    vector<User*> users;
    ifstream inFile("users.dat");
    if (inFile.is_open()) {
        string line;
        while (getline(inFile, line)) {
            size_t pos1 = line.find(',');
            size_t pos2 = line.find(',', pos1+1);
            string username = line.substr(0, pos1);
            string password = line.substr(pos1+1, pos2-pos1-1);
            Role role = static_cast<Role>(stoi(line.substr(pos2+1)));
            if (role == Role::ADMIN) {
                users.push_back(new Admin(username, password));
            } else {
                users.push_back(new RegularUser(username, password));
            }
        }
        inFile.close();
    }
    return users;
}

class PetAdoptionSystem {
private:
    vector<User*> users;
    vector<Pet> pets;
    vector<Application> applications;
    int nextAppID = 1;

    void clearScreen() {
        system("cls || clear");
    }

    string getHiddenInput(const string& prompt) {
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

    string getValidatedInput(const string& prompt,
            bool (*validator)(const string&),
            const string& errorMsg,
            int maxAttempts = 3) {
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
        cout << "Too many failed attempts. Returning to menu.\n";
        return "0";
    }

    int getNumericInput(const string& prompt, int min, int max, int maxAttempts = 3) {
        int value;
        int attempts = 0;
        string input;
        
        while (attempts < maxAttempts) {
            cout << prompt;
            getline(cin, input);
            
            // Check for invalid patterns like "1e", "1 1", etc.
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
        
        cout << "Too many failed attempts. Operation cancelled.\n";
        return 0;
    }

    void browsePets() {
        clearScreen();
        cout << "\n=== AVAILABLE PETS ===\n";
        if (pets.empty()) {
            cout << "No pets available for adoption.\n";
            return;
        }
        for (size_t i = 0; i < pets.size(); ++i) {
            if (!pets[i].isAdopted()) {
                cout << i+1 << ". " << pets[i].getName() << " (" << pets[i].getBreed() 
                     << "), Age: " << pets[i].getAge() 
                     << ", Vaccinated: " << (pets[i].isVaccinated() ? "Yes" : "No") << "\n";
            }
        }
        cout << "\n0. Back\n";
        int choice = getNumericInput("Select pet to apply for adoption (0 to cancel): ", 0, pets.size());
        if (choice == 0) return;
        
        // Process adoption application
        cout << "Application submitted for " << pets[choice-1].getName() << "!\n";
        applications.push_back(Application(nextAppID++, "current_user", pets[choice-1].getName()));
    }

    void checkStatus() {
        clearScreen();
        cout << "\n=== APPLICATION STATUS ===\n";
        if (applications.empty()) {
            cout << "No applications found.\n";
            return;
        }
        for (const auto& app : applications) {
            cout << "ID: " << app.getID() << ", Pet: " << app.getPetName() 
                 << ", Status: " << app.getStatus() << "\n";
        }
    }

    void viewHistory() {
        clearScreen();
        cout << "\n=== ADOPTION HISTORY ===\n";
        bool found = false;
        for (const auto& pet : pets) {
            if (pet.isAdopted()) {
                cout << pet.getName() << " (" << pet.getBreed() << ")\n";
                found = true;
            }
        }
        if (!found) {
            cout << "No adoption history found.\n";
        }
    }

    void managePets() {
        clearScreen();
        cout << "\n=== MANAGE PETS ===\n";
        cout << "1. Add Pet\n";
        cout << "2. Edit Pet\n";
        cout << "3. Delete Pet\n";
        cout << "4. View All Pets\n";
        cout << "0. Back\n";
        int choice = getNumericInput("Enter choice: ", 0, 4);
        
        switch (choice) {
            case 1: {
                clearScreen();
                cout << "\n=== ADD NEW PET ===\n";
                string name = getValidatedInput("Pet name: ", isValidName, "Invalid name");
                if (name == "0") break;
                string breed = getValidatedInput("Breed: ", isValidBreed, "Invalid breed");
                if (breed == "0") break;
                int age = getAgeInput("Age: ");
                bool vaccinated = getNumericInput("Vaccinated? (1=Yes, 0=No): ", 0, 1);
                pets.push_back(Pet(name, breed, age, vaccinated));
                cout << "Pet added successfully!\n";
                break;
            }
            case 2: {
                clearScreen();
                cout << "\n=== EDIT PET ===\n";
                if (pets.empty()) {
                    cout << "No pets available to edit.\n";
                    break;
                }
                for (size_t i = 0; i < pets.size(); ++i) {
                    cout << i+1 << ". " << pets[i].getName() << " (" << pets[i].getBreed() << ")\n";
                }
                int petChoice = getNumericInput("Select pet to edit (0 to cancel): ", 0, pets.size());
                if (petChoice == 0) break;
                
                Pet& pet = pets[petChoice-1];
                cout << "1. Name: " << pet.getName() << "\n";
                cout << "2. Breed: " << pet.getBreed() << "\n";
                cout << "3. Age: " << pet.getAge() << "\n";
                cout << "4. Vaccinated: " << (pet.isVaccinated() ? "Yes" : "No") << "\n";
                cout << "0. Back\n";
                
                int field = getNumericInput("Select field to edit: ", 0, 4);
                if (field == 0) break;
                
                switch (field) {
                    case 1: {
                        string newName = getValidatedInput("New name: ", isValidName, "Invalid name");
                        if (newName != "0") pet.setName(newName);
                        break;
                    }
                    case 2: {
                        string newBreed = getValidatedInput("New breed: ", isValidBreed, "Invalid breed");
                        if (newBreed != "0") pet.setBreed(newBreed);
                        break;
                    }
                    case 3: {
                        int newAge = getAgeInput("New age: ");
                        if (newAge != 0) pet.setAge(newAge);
                        break;
                    }
                    case 4: {
                        int vax = getNumericInput("Vaccinated? (1=Yes, 0=No): ", 0, 1);
                        pet.setVaccinated(vax);
                        break;
                    }
                }
                cout << "Pet updated successfully!\n";
                break;
            }
            case 3: {
                clearScreen();
                cout << "\n=== DELETE PET ===\n";
                if (pets.empty()) {
                    cout << "No pets available to delete.\n";
                    break;
                }
                for (size_t i = 0; i < pets.size(); ++i) {
                    cout << i+1 << ". " << pets[i].getName() << " (" << pets[i].getBreed() << ")\n";
                }
                int petChoice = getNumericInput("Select pet to delete (0 to cancel): ", 0, pets.size());
                if (petChoice == 0) break;
                
                pets.erase(pets.begin() + petChoice - 1);
                cout << "Pet deleted successfully!\n";
                break;
            }
            case 4: {
                clearScreen();
                cout << "\n=== ALL PETS ===\n";
                if (pets.empty()) {
                    cout << "No pets in the system.\n";
                    break;
                }
                for (size_t i = 0; i < pets.size(); ++i) {
                    cout << i+1 << ". " << pets[i].getName() << " (" << pets[i].getBreed() 
                         << "), Age: " << pets[i].getAge() 
                         << ", Vaccinated: " << (pets[i].isVaccinated() ? "Yes" : "No")
                         << ", Status: " << (pets[i].isAdopted() ? "Adopted" : "Available") << "\n";
                }
                break;
            }
        }
    }

    void processApplications() {
        clearScreen();
        cout << "\n=== PROCESS APPLICATIONS ===\n";
        if (applications.empty()) {
            cout << "No applications to process.\n";
            return;
        }
        
        for (size_t i = 0; i < applications.size(); ++i) {
            if (applications[i].getStatus() == "Pending") {
                cout << i+1 << ". ID: " << applications[i].getID() 
                     << ", User: " << applications[i].getUsername() 
                     << ", Pet: " << applications[i].getPetName() << "\n";
            }
        }
        
        int choice = getNumericInput("Select application to process (0 to cancel): ", 0, applications.size());
        if (choice == 0) return;
        
        Application& app = applications[choice-1];
        cout << "1. Approve\n";
        cout << "2. Reject\n";
        cout << "0. Back\n";
        int action = getNumericInput("Enter action: ", 0, 2);
        
        if (action == 1) {
            app.approve();
            // Find the pet and mark as adopted
            for (auto& pet : pets) {
                if (pet.getName() == app.getPetName()) {
                    pet.markAsAdopted();
                    break;
                }
            }
            cout << "Application approved!\n";
        } else if (action == 2) {
            app.reject();
            cout << "Application rejected.\n";
        }
    }

    void searchPets() {
        clearScreen();
        cout << "\n=== SEARCH PETS ===\n";
        cout << "1. By Name\n";
        cout << "2. By Breed\n";
        cout << "3. By Age Range\n";
        cout << "0. Back\n";
        int choice = getNumericInput("Enter choice: ", 0, 3);
        
        if (pets.empty()) {
            cout << "No pets in the system.\n";
            return;
        }
        
        vector<Pet> results;
        switch (choice) {
            case 1: {
                string name = getValidatedInput("Enter pet name to search: ", isValidName, "Invalid name");
                if (name == "0") break;
                for (const auto& pet : pets) {
                    if (pet.getName().find(name) != string::npos) {
                        results.push_back(pet);
                    }
                }
                break;
            }
            case 2: {
                string breed = getValidatedInput("Enter breed to search: ", isValidBreed, "Invalid breed");
                if (breed == "0") break;
                for (const auto& pet : pets) {
                    if (pet.getBreed().find(breed) != string::npos) {
                        results.push_back(pet);
                    }
                }
                break;
            }
            case 3: {
                int minAge = getNumericInput("Enter minimum age: ", 0, 30);
                int maxAge = getNumericInput("Enter maximum age: ", minAge, 30);
                for (const auto& pet : pets) {
                    if (pet.getAge() >= minAge && pet.getAge() <= maxAge) {
                        results.push_back(pet);
                    }
                }
                break;
            }
            case 0:
                return;
        }
        
        if (results.empty()) {
            cout << "No matching pets found.\n";
        } else {
            cout << "\n=== SEARCH RESULTS ===\n";
            for (size_t i = 0; i < results.size(); ++i) {
                cout << i+1 << ". " << results[i].getName() << " (" << results[i].getBreed() 
                     << "), Age: " << results[i].getAge() 
                     << ", Vaccinated: " << (results[i].isVaccinated() ? "Yes" : "No")
                     << ", Status: " << (results[i].isAdopted() ? "Adopted" : "Available") << "\n";
            }
        }
    }

    void runUserDashboard(RegularUser* user) {
        int choice;
        do {
            clearScreen();
            user->showDashboard();
            choice = getNumericInput("Enter choice: ", 1, 4);
            switch (choice) {
                case 1: browsePets(); break;
                case 2: checkStatus(); break;
                case 3: viewHistory(); break;
                case 4: cout << "Logging out...\n"; break;
            }
            if (choice != 4) { cout << "\nPress Enter to continue..."; cin.ignore(); cin.get(); }
        } while (choice != 4);
    }

    void addAdmin() {
        clearScreen();
        cout << "\n=== ADD NEW ADMIN ===\n";
        string username = getValidatedInput(
            "Admin username (4-20 chars, case-sensitive): ", isValidUsername, "Invalid username format");
        if (username == "0") return;
        for (auto user : users)
            if (user->getUsername() == username) {
                cout << "Username exists!\n";
                return;
            }
        string password = getHiddenInput("Password: ");
        if (!isValidPassword(password)) {
            cout << "Invalid password!\n";
            return;
        }
        users.push_back(new Admin(username, password));
        saveUsersToFile(users);
        cout << "Admin added!\n";
    }

    void manageUserAccounts() {
        clearScreen();
        cout << "\n=== MANAGE USER ACCOUNTS ===\n";
        
        // Create sorted lists
        vector<User*> allUsers(users.begin(), users.end());
        sort(allUsers.begin(), allUsers.end(), 
            [](User* a, User* b) { return a->getUsername() < b->getUsername(); });
        
        // Display users
        int count = 0;
        for (size_t i = 0; i < allUsers.size(); ++i) {
            cout << i+1 << ". " << allUsers[i]->getUsername() 
                 << " (" << (allUsers[i]->getRole() == Role::ADMIN ? "Admin" : "User") << ")\n";
            count++;
        }
        
        if (count == 0) { 
            cout << "No users found.\n"; 
            return; 
        }
        
        cout << "0. Back\n";
        int idx = getNumericInput("Select user to Edit/Delete (0 to cancel): ", 0, allUsers.size()) - 1;
        if (idx == -1) return;
        
        User* user = allUsers[idx];
        cout << "1. Edit Username\n2. Edit Password\n3. Delete User\n0. Back\n";
        int choice = getNumericInput("Enter choice: ", 0, 3);
        if (choice == 0) return;
        
        switch (choice) {
            case 1: {
                string newName = getValidatedInput("New username: ", isValidUsername, "Invalid username");
                if (newName != "0") user->username = newName;
                cout << "Username updated!\n";
                break;
            }
            case 2: {
                string newPwd = getHiddenInput("New password: ");
                if (newPwd != "0") user->password = newPwd;
                cout << "Password updated!\n";
                break;
            }
            case 3: {
                auto it = find(users.begin(), users.end(), user);
                if (it != users.end()) users.erase(it);
                cout << "User deleted!\n";
                break;
            }
        }
        saveUsersToFile(users);
    }

    void registerUser() {
        clearScreen();
        cout << "\n=== REGISTRATION ===\n";
        cout << "Note: Only regular user registration is allowed\n";
        cout << "Admin accounts must be created by system administrators\n\n";
        string username = getValidatedInput(
            "Enter username (4-20 alphanumeric chars, '0' to cancel): ",
            isValidUsername,
            "Invalid username format");
        if (username == "0") return;
        for (auto user : users) {
            if (user->getUsername() == username) {
                cout << "Username already exists!\n";
                cout << "1. Try another username\n";
                cout << "0. Back to menu\n";
                int choice = getNumericInput("Enter choice: ", 0, 1);
                if (choice == 1) registerUser();
                return;
            }
        }
        string password = getHiddenInput("Enter password: ");
        if (password == "0") return;
        if (!isValidPassword(password)) {
            cout << "Invalid password!\n";
            cout << "1. Try again\n";
            cout << "0. Back to menu\n";
            int choice = getNumericInput("Enter choice: ", 0, 1);
            if (choice == 1) registerUser();
            return;
        }
        users.push_back(new RegularUser(username, password));
        saveUsersToFile(users);
        cout << "Registration successful!\n";
    }

    User* login() {
        clearScreen();
        cout << "\n=== LOGIN ===\n";
        cout << "Select role:\n";
        cout << "1. Admin\n";
        cout << "2. User\n";
        cout << "0. Back to main menu\n";
        int roleChoice = getNumericInput("Enter choice: ", 0, 2);
        if (roleChoice == 0) return nullptr;
        Role selectedRole = (roleChoice == 1) ? Role::ADMIN : Role::USER;
        if (selectedRole == Role::ADMIN) {
            // Default admin shortcut
            for (auto user : users) {
                if (user->getRole() == Role::ADMIN &&
                    user->getUsername() == "admin" &&
                    user->getPassword() == "admin123") {
                    cout << "\nDefault admin login successful!\n";
                    return user;
                }
            }
            cout << "Default admin not found.\n";
            return nullptr;
        }
        // User login flow as usual
        string username = getValidatedInput(
            "Username (or '0' to cancel): ",
            [](const string& s) { return isValidUsername(s) || s == "0"; },
            "Invalid username format");
        if (username == "0") return nullptr;
        string password = getHiddenInput("Password (or '0' to cancel): ");
        if (password == "0") return nullptr;
        for (auto user : users) {
            if (user->getRole() == selectedRole &&
                user->authenticate(username, password)) {
                cout << "\nLogin successful!\n";
                return user;
            }
        }
        cout << "\nInvalid credentials or role mismatch.\n";
        cout << "1. Try again\n";
        cout << "0. Back to menu\n";
        int choice = getNumericInput("Enter choice: ", 0, 1);
        return (choice == 1) ? login() : nullptr;
    }

    void runAdminDashboard(Admin* admin) {
        int choice;
        do {
            clearScreen();
            admin->showDashboard();
            choice = getNumericInput("Enter choice: ", 1, 6);
            switch (choice) {
                case 1: addAdmin(); break;
                case 2: manageUserAccounts(); break;
                case 3: managePets(); break;
                case 4: processApplications(); break;
                case 5: searchPets(); break;
                case 6: cout << "Logging out...\n"; break;
            }
            if (choice != 6) { cout << "\nPress Enter to continue..."; cin.ignore(); cin.get(); }
        } while (choice != 6);
    }

public:
    PetAdoptionSystem() {
        users = loadUsersFromFile();
        if (users.empty()) {
            users.push_back(new Admin("admin", "admin123"));
            saveUsersToFile(users);
        }
        pets.push_back(Pet("Whiskers", "Siamese", 2, true));
        pets.push_back(Pet("Rex", "Labrador", 3, true));
    }

    ~PetAdoptionSystem() {
        saveUsersToFile(users);
        for (auto user : users) {
            delete user;
        }
    }

    void run() {
        int choice;
        do {
            clearScreen();
            cout << "\n=== PET ADOPTION SYSTEM ===\n";
            cout << "1. Login\n";
            cout << "2. Register\n";
            cout << "3. Exit\n";
            choice = getNumericInput("Enter choice: ", 1, 3);
            
            switch (choice) {
                case 1: {
                    User* user = login();
                    if (user) {
                        if (user->getRole() == Role::ADMIN) {
                            runAdminDashboard(static_cast<Admin*>(user));
                        } else {
                            runUserDashboard(static_cast<RegularUser*>(user));
                        }
                    }
                    break;
                }
                case 2:
                    registerUser();
                    break;
                case 3:
                    cout << "Exiting system...\n";
                    break;
            }
            if (choice != 3) { cout << "\nPress Enter to continue..."; cin.ignore(); cin.get(); }
        } while (choice != 3);
    }
};

int main() {
    PetAdoptionSystem system;
    system.run();
    return 0;
}




/* I still want to keep the // Updated validation functions
bool isValidUsername(const string& username) {
    // 4-20 chars, letters, numbers, and spaces allowed
    if (username.length() < 4 || username.length() > 20) return false;
    if (!regex_match(username, regex("^[A-Za-z0-9 ]+$"))) return false;
    // No consecutive spaces
    for (size_t i = 1; i < username.size(); ++i)
        if (isspace(username[i]) && isspace(username[i-1])) return false;
    return true;
}

bool isValidPassword(const string& password) {
    // No restrictions on password
    return !password.empty();
}

bool isValidName(const string& name) {
    if (name.empty()) return false;
    // Allow letters, numbers, and spaces
    if (!regex_match(name, regex("^[A-Za-z0-9 ]+$"))) return false;
    // No consecutive spaces
    for (size_t i = 1; i < name.size(); ++i)
        if (isspace(name[i]) && isspace(name[i-1])) return false;
    return true;
}

// Updated age input function
int getAgeInput(const string& prompt) {
    string input;
    while (true) {
        cout << prompt;
        getline(cin, input);
        
        // Check for simple numeric input
        if (regex_match(input, regex("^\\d+$"))) {
            return stoi(input);
        }
        // Check for "X years" or "X months" format
        smatch matches;
        if (regex_match(input, matches, regex("^(\\d+)\\s*(years?|months?)$"))) {
            int value = stoi(matches[1].str());
            string unit = matches[2].str();
            if (unit.find("month") != string::npos) {
                return value / 12; // Convert months to years
            }
            return value;
        }
        cout << "Invalid age format. Please enter like '2', '3 years', or '6 months'\n";
    }
}

// Updated admin login check
User* login() {
    clearScreen();
    cout << "\n=== LOGIN ===\n";
    cout << "Select role:\n";
    cout << "1. Admin\n";
    cout << "2. User\n";
    cout << "0. Back to main menu\n";
    int roleChoice = getNumericInput("Enter choice: ", 0, 2);
    if (roleChoice == 0) return nullptr;
    
    Role selectedRole = (roleChoice == 1) ? Role::ADMIN : Role::USER;
    string username, password;
    
    if (selectedRole == Role::ADMIN) {
        username = getValidatedInput(
            "Admin username: ",
            isValidUsername,
            "Invalid username format");
        if (username == "0") return nullptr;
        
        password = getHiddenInput("Password: ");
        if (password == "0") return nullptr;
        
        // Check for default admin
        if (username == "admin" && password == "admin123") {
            // Find or create default admin
            for (auto user : users) {
                if (user->getUsername() == "admin") {
                    cout << "\nDefault admin login successful!\n";
                    return user;
                }
            }
            // Create default admin if not found
            Admin* defaultAdmin = new Admin("admin", "admin123");
            users.push_back(defaultAdmin);
            saveUsersToFile(users);
            cout << "\nDefault admin login successful!\n";
            return defaultAdmin;
        }
    }
    
    // Regular user login flow
    username = getValidatedInput(
        "Username: ",
        isValidUsername,
        "Invalid username format");
    if (username == "0") return nullptr;
    
    password = getHiddenInput("Password: ");
    if (password == "0") return nullptr;
    
    for (auto user : users) {
        if (user->getRole() == selectedRole &&
            user->authenticate(username, password)) {
            cout << "\nLogin successful!\n";
            return user;
        }
    }
    
    cout << "\nInvalid credentials or role mismatch.\n";
    cout << "1. Try again\n";
    cout << "0. Back to menu\n";
    int choice = getNumericInput("Enter choice: ", 0, 1);
    return (choice == 1) ? login() : nullptr;
}

// Updated user management to show sorted lists
void manageUserAccounts() {
    clearScreen();
    cout << "\n=== MANAGE USER ACCOUNTS ===\n";
    
    // Create sorted lists
    vector<User*> allUsers(users.begin(), users.end());
    sort(allUsers.begin(), allUsers.end(), 
        [](User* a, User* b) { return a->getUsername() < b->getUsername(); });
    
    // Display users
    int count = 0;
    for (size_t i = 0; i < allUsers.size(); ++i) {
        cout << i+1 << ". " << allUsers[i]->getUsername() 
             << " (" << (allUsers[i]->getRole() == Role::ADMIN ? "Admin" : "User") << ")\n";
        count++;
    }
    
    if (count == 0) { 
        cout << "No users found.\n"; 
        return; 
    }
    
    cout << "0. Back\n";
    int idx = getNumericInput("Select user to Edit/Delete (0 to cancel): ", 0, allUsers.size()) - 1;
    if (idx == -1) return;
    
    User* user = allUsers[idx];
    cout << "1. Edit Username\n2. Edit Password\n3. Delete User\n0. Back\n";
    int choice = getNumericInput("Enter choice: ", 0, 3);
    if (choice == 0) return;
    
    switch (choice) {
        case 1: {
            string newName = getValidatedInput("New username: ", isValidUsername, "Invalid username");
            if (newName != "0") user->username = newName;
            cout << "Username updated!\n";
            break;
        }
        case 2: {
            string newPwd = getHiddenInput("New password: ");
            if (newPwd != "0") user->password = newPwd;
            cout << "Password updated!\n";
            break;
        }
        case 3: {
            auto it = find(users.begin(), users.end(), user);
            if (it != users.end()) users.erase(it);
            cout << "User deleted!\n";
            break;
        }
    }
    saveUsersToFile(users);
}

// Updated numeric input validation
int getNumericInput(const string& prompt, int min, int max, int maxAttempts = 3) {
    int value;
    int attempts = 0;
    string input;
    
    while (attempts < maxAttempts) {
        cout << prompt;
        getline(cin, input);
        
        // Check for invalid patterns like "1e", "1 1", etc.
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
*/
    
    cout << "Too many failed attempts. Operation cancelled.\n";
    return 0;
}

that you gave to me is it possible to still paste this in the code?
