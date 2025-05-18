#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <limits>
#include <cctype>
#include <regex>
#include <conio.h> // For _getch() on Windows
#include <stdlib.h> // For system() commands

using namespace std;

// User roles
enum class Role { ADMIN, USER };

// Input validation functions
bool isValidUsername(const string& username) {
    regex pattern("^[a-zA-Z0-9_]{4,20}$");
    return regex_match(username, pattern);
}

bool isValidPassword(const string& password) {
    return password.length() >= 8 && 
           any_of(password.begin(), password.end(), ::isalpha) &&
           any_of(password.begin(), password.end(), ::isdigit);
}

bool isValidName(const string& name) {
    return !name.empty() && 
           all_of(name.begin(), name.end(), [](char c) {
               return isalpha(c) || isspace(c);
           });
}

bool isValidBreed(const string& breed) {
    return !breed.empty() && 
           all_of(breed.begin(), breed.end(), [](char c) {
               return isalpha(c) || isspace(c);
           });
}

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
};

// Admin class
class Admin : public User {
public:
    Admin(string uname, string pwd) : User(uname, pwd, Role::ADMIN) {}
    
    void showDashboard() override {
        cout << "\n==== ADMIN DASHBOARD ====\n";
        cout << "1. Manage Pet Records\n";
        cout << "2. Process Applications\n";
        cout << "3. Search Pets\n";
        cout << "4. Logout\n";
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

// File handling functions
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
        
        while (attempts < maxAttempts) {
            cout << prompt;
            if (cin >> value) {
                if (value >= min && value <= max) {
                    return value;
                }
                cout << "Please enter between " << min << " and " << max 
                     << " (" << maxAttempts - attempts - 1 << " attempts left)\n";
            } else {
                cout << "Invalid input. Please enter a number ("
                     << maxAttempts - attempts - 1 << " attempts left)\n";
                cin.clear();
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            attempts++;
        }
        
        cout << "Too many failed attempts. Operation cancelled.\n";
        return 0;
    }

public:
    PetAdoptionSystem() {
        users = loadUsersFromFile();
        if (users.empty()) {
            users.push_back(new Admin("admin", "admin123"));
            saveUsersToFile(users);
        }
        
        // Sample pets
        pets.push_back(Pet("Whiskers", "Siamese", 2, true));
        pets.push_back(Pet("Rex", "Labrador", 3, true));
    }
    
    ~PetAdoptionSystem() {
        saveUsersToFile(users);
        for (auto user : users) {
            delete user;
        }
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
        
        string password = getHiddenInput(
            "Enter password (min 8 chars with letters and numbers, '0' to cancel): ");
        if (password == "0") return;
        
        if (!isValidPassword(password)) {
            cout << "Password doesn't meet requirements.\n";
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
            
            choice = getNumericInput("Enter choice: ", 1, 4);
            if (choice == 0) continue;
            
            switch (choice) {
                case 1: managePets(); break;
                case 2: processApplications(); break;
                case 3: searchPets(); break;
                case 4: cout << "Logging out...\n"; break;
            }
            
            if (choice != 4) {
                cout << "\nPress Enter to continue...";
                cin.ignore();
                cin.get();
            }
        } while (choice != 4);
    }
    
    void runUserDashboard(RegularUser* user) {
        int choice;
        do {
            clearScreen();
            user->showDashboard();
            
            choice = getNumericInput("Enter choice: ", 1, 4);
            if (choice == 0) continue;
            
            switch (choice) {
                case 1: browsePets(user); break;
                case 2: checkStatus(user); break;
                case 3: viewHistory(user); break;
                case 4: cout << "Logging out...\n"; break;
            }
            
            if (choice != 4) {
                cout << "\nPress Enter to continue...";
                cin.ignore();
                cin.get();
            }
        } while (choice != 4);
    }
    
    void managePets() {
        int choice;
        do {
            clearScreen();
            cout << "\n=== MANAGE PETS ===\n";
            cout << "1. Add Pet\n";
            cout << "2. Edit Pet\n";
            cout << "3. Delete Pet\n";
            cout << "0. Back\n";
            
            choice = getNumericInput("Enter choice: ", 0, 3);
            if (choice == 0) continue;
            
            switch (choice) {
                case 1: addPet(); break;
                case 2: editPet(); break;
                case 3: deletePet(); break;
            }
            
            if (choice != 0) {
                cout << "\nPress Enter to continue...";
                cin.ignore();
                cin.get();
            }
        } while (choice != 0);
    }
    
    void addPet() {
        clearScreen();
        cout << "\n=== ADD NEW PET ===\n";
        
        string name = getValidatedInput(
            "Pet name (or '0' to cancel): ",
            isValidName,
            "Invalid name (letters and spaces only)");
        if (name == "0") return;
        
        string breed = getValidatedInput(
            "Breed (or '0' to cancel): ",
            isValidBreed,
            "Invalid breed name");
        if (breed == "0") return;
        
        int age = getNumericInput(
            "Age (1-30, 0 to cancel): ", 0, 30);
        if (age == 0) return;
        
        char vax;
        cout << "Is the pet vaccinated? (y/n, 0 to cancel): ";
        cin >> vax;
        if (vax == '0') return;
        bool vaccinated = (tolower(vax) == 'y';
        
        pets.push_back(Pet(name, breed, age, vaccinated));
        cout << "\nPet added successfully!\n";
    }
    
    void editPet() {
        clearScreen();
        cout << "\n=== EDIT PET ===\n";
        
        if (pets.empty()) {
            cout << "No pets available to edit.\n";
            return;
        }
        
        for (size_t i = 0; i < pets.size(); i++) {
            cout << i+1 << ". " << pets[i].getName() 
                 << " (" << pets[i].getBreed() << ")\n";
        }
        
        int petIndex = getNumericInput(
            "Select pet to edit (0 to cancel): ", 0, pets.size()) - 1;
        if (petIndex == -1) return;
        
        Pet& pet = pets[petIndex];
        
        cout << "\nCurrent details:\n";
        cout << "Name: " << pet.getName() << endl;
        cout << "Breed: " << pet.getBreed() << endl;
        cout << "Age: " << pet.getAge() << endl;
        cout << "Vaccinated: " << (pet.isVaccinated() ? "Yes" : "No") << endl;
        
        cout << "\n=== EDIT OPTIONS ===\n";
        cout << "1. Change name\n";
        cout << "2. Change breed\n";
        cout << "3. Change age\n";
        cout << "4. Toggle vaccination status\n";
        cout << "0. Back\n";
        
        int choice = getNumericInput("Enter choice: ", 0, 4);
        if (choice == 0) return;
        
        switch (choice) {
            case 1: {
                string newName = getValidatedInput(
                    "New name (or '0' to cancel): ",
                    isValidName,
                    "Invalid name");
                if (newName == "0") break;
                pet = Pet(newName, pet.getBreed(), pet.getAge(), pet.isVaccinated());
                cout << "\nName updated successfully!\n";
                break;
            }
            case 2: {
                string newBreed = getValidatedInput(
                    "New breed (or '0' to cancel): ",
                    isValidBreed,
                    "Invalid breed");
                if (newBreed == "0") break;
                pet = Pet(pet.getName(), newBreed, pet.getAge(), pet.isVaccinated());
                cout << "\nBreed updated successfully!\n";
                break;
            }
            case 3: {
                int newAge = getNumericInput(
                    "New age (1-30, 0 to cancel): ", 0, 30);
                if (newAge == 0) break;
                pet.setAge(newAge);
                cout << "\nAge updated successfully!\n";
                break;
            }
            case 4:
                pet.setVaccinated(!pet.isVaccinated());
                cout << "\nVaccination status toggled.\n";
                break;
        }
    }
    
    void deletePet() {
        clearScreen();
        cout << "\n=== DELETE PET ===\n";
        
        if (pets.empty()) {
            cout << "No pets available to delete.\n";
            return;
        }
        
        for (size_t i = 0; i < pets.size(); i++) {
            cout << i+1 << ". " << pets[i].getName() 
                 << " (" << pets[i].getBreed() << ")\n";
        }
        
        int petIndex = getNumericInput(
            "Select pet to delete (0 to cancel): ", 0, pets.size()) - 1;
        if (petIndex == -1) return;
        
        string petName = pets[petIndex].getName();
        pets.erase(pets.begin() + petIndex);
        
        applications.erase(
            remove_if(applications.begin(), applications.end(),
                [&petName](const Application& app) {
                    return app.getPetName() == petName;
                }),
            applications.end());
        
        cout << "\nPet deleted successfully!\n";
    }
    
    void processApplications() {
        clearScreen();
        cout << "\n=== PROCESS APPLICATIONS ===\n";
        
        if (applications.empty()) {
            cout << "No applications to process.\n";
            return;
        }
        
        bool pendingFound = false;
        for (auto& app : applications) {
            if (app.getStatus() == "Pending") {
                pendingFound = true;
                cout << "\nApplication ID: " << app.getID() << endl;
                cout << "Pet: " << app.getPetName() << endl;
                cout << "Applicant: " << app.getUsername() << endl;
                
                cout << "1. Approve\n";
                cout << "2. Reject\n";
                cout << "0. Cancel\n";
                
                int decision = getNumericInput("Enter choice: ", 0, 2);
                if (decision == 0) {
                    cout << "Processing cancelled.\n";
                    return;
                }
                
                if (decision == 1) {
                    app.approve();
                    for (auto& pet : pets) {
                        if (pet.getName() == app.getPetName()) {
                            pet.markAsAdopted();
                            break;
                        }
                    }
                    cout << "Application approved!\n";
                } else {
                    app.reject();
                    cout << "Application rejected.\n";
                }
            }
        }
        
        if (!pendingFound) {
            cout << "No pending applications found.\n";
        }
    }
    
    void searchPets() {
        clearScreen();
        cout << "\n=== SEARCH PETS ===\n";
        cout << "1. By breed\n";
        cout << "2. By age range\n";
        cout << "3. By vaccination status\n";
        cout << "0. Back\n";
        
        int choice = getNumericInput("Enter choice: ", 0, 3);
        if (choice == 0) return;
        
        vector<Pet> results;
        
        switch (choice) {
            case 1: {
                string breed = getValidatedInput(
                    "Enter breed to search (or '0' to cancel): ",
                    isValidBreed,
                    "Invalid breed name");
                if (breed == "0") return;
                
                transform(breed.begin(), breed.end(), breed.begin(), ::tolower);
                for (const auto& pet : pets) {
                    string petBreed = pet.getBreed();
                    transform(petBreed.begin(), petBreed.end(), petBreed.begin(), ::tolower);
                    if (petBreed.find(breed) != string::npos) {
                        results.push_back(pet);
                    }
                }
                break;
            }
            case 2: {
                int minAge = getNumericInput(
                    "Minimum age (0 to cancel): ", 0, 30);
                if (minAge == 0) return;
                
                int maxAge = getNumericInput(
                    "Maximum age (" + to_string(minAge) + "-30, 0 to cancel): ", 
                    minAge, 30);
                if (maxAge == 0) return;
                
                for (const auto& pet : pets) {
                    if (pet.getAge() >= minAge && pet.getAge() <= maxAge) {
                        results.push_back(pet);
                    }
                }
                break;
            }
            case 3: {
                cout << "1. Show vaccinated\n";
                cout << "2. Show unvaccinated\n";
                cout << "0. Cancel\n";
                
                int vaxChoice = getNumericInput("Enter choice: ", 0, 2);
                if (vaxChoice == 0) return;
                
                bool vaccinated = (vaxChoice == 1);
                for (const auto& pet : pets) {
                    if (pet.isVaccinated() == vaccinated) {
                        results.push_back(pet);
                    }
                }
                break;
            }
        }
        
        cout << "\n=== SEARCH RESULTS ===\n";
        if (results.empty()) {
            cout << "No pets found matching your criteria.\n";
        } else {
            for (const auto& pet : results) {
                cout << "Name: " << pet.getName() << endl;
                cout << "Breed: " << pet.getBreed() << endl;
                cout << "Age: " << pet.getAge() << endl;
                cout << "Vaccinated: " << (pet.isVaccinated() ? "Yes" : "No") << endl;
                cout << "Status: " << (pet.isAdopted() ? "Adopted" : "Available") << endl;
                cout << "-------------------\n";
            }
        }
    }
    
    void browsePets(RegularUser* user) {
        clearScreen();
        cout << "\n=== AVAILABLE PETS ===\n";
        
        bool availableFound = false;
        for (const auto& pet : pets) {
            if (!pet.isAdopted()) {
                availableFound = true;
                cout << "Name: " << pet.getName() << endl;
                cout << "Breed: " << pet.getBreed() << endl;
                cout << "Age: " << pet.getAge() << endl;
                cout << "Vaccinated: " << (pet.isVaccinated() ? "Yes" : "No") << endl;
                cout << "-------------------\n";
            }
        }
        
        if (!availableFound) {
            cout << "No pets currently available for adoption.\n";
            return;
        }
        
        cout << "1. Adopt a pet\n";
        cout << "0. Back\n";
        
        int choice = getNumericInput("Enter choice: ", 0, 1);
        if (choice == 0) return;
        
        string petName = getValidatedInput(
            "Enter the name of the pet you want to adopt (or '0' to cancel): ",
            isValidName,
            "Invalid pet name");
        
        if (petName == "0") return;
        
        bool petFound = false;
        for (const auto& pet : pets) {
            if (pet.getName() == petName && !pet.isAdopted()) {
                petFound = true;
                break;
            }
        }
        
        if (!petFound) {
            cout << "Pet not found or already adopted.\n";
            return;
        }
        
        applications.push_back(Application(nextAppID++, user->getUsername(), petName));
        cout << "\nApplication submitted successfully!\n";
        cout << "Your application ID is: " << nextAppID-1 << endl;
    }
    
    void checkStatus(RegularUser* user) {
        clearScreen();
        cout << "\n=== APPLICATION STATUS ===\n";
        
        bool found = false;
        for (const auto& app : applications) {
            if (app.getUsername() == user->getUsername()) {
                found = true;
                cout << "Application ID: " << app.getID() << endl;
                cout << "Pet: " << app.getPetName() << endl;
                cout << "Status: " << app.getStatus() << endl;
                cout << "-------------------\n";
            }
        }
        
        if (!found) {
            cout << "No applications found.\n";
        }
    }
    
    void viewHistory(RegularUser* user) {
        checkStatus(user);
    }
    
    void run() {
        while (true) {
            clearScreen();
            cout << "=== PET ADOPTION SYSTEM ===\n";
            cout << "1. Login\n";
            cout << "2. Register\n";
            cout << "3. Exit\n";
            
            int choice = getNumericInput("Enter choice: ", 1, 3);
            
            switch (choice) {
                case 1: {
                    User* user = nullptr;
                    int attempts = 0;
                    const int maxAttempts = 3;
                    
                    while (attempts < maxAttempts && !user) {
                        user = login();
                        if (!user) {
                            attempts++;
                            if (attempts < maxAttempts) {
                                cout << "Please try again (" 
                                     << maxAttempts - attempts 
                                     << " attempts remaining).\n";
                                cout << "Press Enter to continue...";
                                cin.ignore();
                                cin.get();
                            }
                        }
                    }
                    
                    if (user) {
                        if (user->getRole() == Role::ADMIN) {
                            runAdminDashboard(dynamic_cast<Admin*>(user));
                        } else {
                            runUserDashboard(dynamic_cast<RegularUser*>(user));
                        }
                    } else {
                        cout << "Maximum login attempts reached.\n";
                        cout << "Press Enter to continue...";
                        cin.ignore();
                        cin.get();
                    }
                    break;
                }
                case 2:
                    registerUser();
                    break;
                case 3:
                    cout << "Exiting system...\n";
                    return;
            }
        }
    }
};

int main() {
    PetAdoptionSystem system;
    system.run();
    return 0;
}