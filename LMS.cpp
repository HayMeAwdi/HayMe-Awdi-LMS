#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <ctime>
#include <sstream>
using namespace std;


void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pauseScreen() {
    cout << "\nPlease enter to continue...";
    cin.ignore();
    cin.get();
}

void printBanner() {
    time_t now = time(0);
    tm *ltm = localtime(&now);

    char dateStr[20], timeStr[20];
    strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", ltm);
    strftime(timeStr, sizeof(timeStr), "%I:%M %p", ltm);

    cout << "===============================================\n";
    cout << "      HayMe Awdi Library Management System     \n";
    cout << "===============================================\n";
    cout << "Date: " << dateStr << "   Time: " << timeStr << "\n\n";
}

class Book {
private:
    string title, author, isbn;
    bool isAvailable;
    string borrowedByID;
public:
    Book(string t, string a, string i) : title(t), author(a), isbn(i), isAvailable(true), borrowedByID("") {}

    string getTitle() { return title; }
    string getAuthor() { return author; }
    string getISBN() { return isbn; }
    bool getAvailability() { return isAvailable; }
    string getBorrowedByID() { return borrowedByID; }

    void setAvailability(bool status, string userID = "") {
        isAvailable = status;
        borrowedByID = userID;
    }

    void save(ofstream &out) {
        out << title << "|" << author << "|" << isbn << "|" << isAvailable << "|" << borrowedByID << "\n";
    }

    static Book load(string line) {
        string t, a, i, borrower;
        bool status;
        size_t pos1 = line.find("|");
        size_t pos2 = line.find("|", pos1 + 1);
        size_t pos3 = line.find("|", pos2 + 1);
        size_t pos4 = line.find("|", pos3 + 1);

        t = line.substr(0, pos1);
        a = line.substr(pos1 + 1, pos2 - pos1 - 1);
        i = line.substr(pos2 + 1, pos3 - pos2 - 1);
        status = stoi(line.substr(pos3 + 1, pos4 - pos3 - 1));
        borrower = line.substr(pos4 + 1);

        Book b(t, a, i);
        b.setAvailability(status, borrower);
        return b;
    }
};

class LibraryUser {
private:
    string userID, name;
    vector<string> borrowedBooks;
public:
    LibraryUser(string id, string n) : userID(id), name(n) {}

    string getUserID() { return userID; }
    string getName() { return name; }
    void setName(string n) { name = n; }
    vector<string> getBorrowedBooks() { return borrowedBooks; }

    void borrowBook(string isbn) { borrowedBooks.push_back(isbn); }
    void returnBook(string isbn) {
        borrowedBooks.erase(remove(borrowedBooks.begin(), borrowedBooks.end(), isbn), borrowedBooks.end());
    }

    void save(ofstream &out) {
        out << userID << "|" << name;
        for (string &isbn : borrowedBooks) out << "|" << isbn;
        out << "\n";
    }


    static LibraryUser load(string line) {
        stringstream ss(line);
        string segment;
        vector<string> segments;
        while(getline(ss, segment, '|')) {
           segments.push_back(segment);
        }

        LibraryUser u(segments[0], segments[1]);


        for (size_t i = 2; i < segments.size(); ++i) {
            if (!segments[i].empty()) {
                u.borrowBook(segments[i]);
            }
        }
        return u;
    }
};

class Library {
private:
    vector<Book> books;
    vector<LibraryUser> users;
public:
    Library() { loadFromFiles(); preloadBooks(); }

    void preloadBooks() {
        if (books.empty()) {
            addBook("Venom", "Ruben Samuel Fleischer", "01");
            addBook("Boruto: Two Blue Vortex", "Ukyō Kodachi", "02");
            addBook("The Wild Robot", "Christopher Michael Sanders", "03");
        }
    }

    bool bookExists(string isbn) {
        for (Book &b : books) if (b.getISBN() == isbn) return true;
        return false;
    }

    bool userExists(string id) {
        for (LibraryUser &u : users) if (u.getUserID() == id) return true;
        return false;
    }


    bool userNameExists(string name) {
        for (LibraryUser &u : users) if (u.getName() == name) return true;
        return false;
    }

    Book* findBook(string isbn) {
        for (auto &b : books) if (b.getISBN() == isbn) return &b;
        return nullptr;
    }

    LibraryUser* findUser(string id) {
        for (auto &u : users) if (u.getUserID() == id) return &u;
        return nullptr;
    }

    int getUsersCount() { return users.size(); }
    int getBooksCount() { return books.size(); }

    void addBook(string t, string a, string i) {
        if (bookExists(i)) { cout << "Failed: Book with ISBN " << i << " already exists.\n"; return; }
        books.push_back(Book(t, a, i));
        saveToFiles();
        cout << "Successfully added: " << t << " by " << a << " (" << i << ")\n";
    }

    void removeBookByIndex(int index) {
        if (books.empty()) {
            cout << "No books available.\n";
            return;
        }
        if (index < 1 || index > (int)books.size()) { cout << "Failed: Invalid selection.\n"; return; }
        auto it = books.begin() + (index - 1);
        char confirm;
        cout << "\nAre you sure you want to remove \"" << it->getTitle() << "\" by " << it->getAuthor() << " (" << it->getISBN() << ")? (Y/N): ";
        cin >> confirm;
        if (confirm == 'Y' || confirm == 'y') {
            cout << "Successfully removed: " << it->getTitle() << " by " << it->getAuthor() << " (" << it->getISBN() << ")\n";
            books.erase(it);
            saveToFiles();
        } else {
            cout << "Cancelled: Book not removed.\n";
        }
    }

    void registerUser(string id, string n) {
        if (userExists(id)) { cout << "Failed: User with ID " << id << " already exists.\n"; return; }

        if (userNameExists(n)) {
            cout << "Failed: A user with the name \"" << n << "\" already exists.\n";
            return;
        }
        users.push_back(LibraryUser(id, n));
        saveToFiles();
        cout << "Successfully registered: " << n << " (" << id << ")\n";
    }

    void editUserByIndex(int index, string newName) {
        if (users.empty() || index < 1 || index > (int)users.size()) {
             cout << "Failed: Invalid selection.\n";
             return;
        }

        if (userNameExists(newName)) {
            cout << "Failed: A user with the name \"" << newName << "\" already exists.\n";
            return;
        }
        users[index - 1].setName(newName);
        saveToFiles();
        cout << "Successfully updated user: " << users[index - 1].getUserID() << " - " << newName << "\n";
    }

    void removeUserByIndex(int index) {
        if (users.empty()) {
            cout << "No users registered.\n";
            return;
        }
        if (index < 1 || index > (int)users.size()) { cout << "Failed: Invalid selection.\n"; return; }
        auto it = users.begin() + (index - 1);
        char confirm;
        cout << "\nAre you sure you want to remove " << it->getUserID() << " - " << it->getName() << "? (Y/N): ";
        cin >> confirm;
        if (confirm == 'Y' || confirm == 'y') {
            cout << "Successfully removed user: " << it->getUserID() << " - " << it->getName() << "\n";
            users.erase(it);
            saveToFiles();
        } else {
            cout << "Cancelled: User not removed.\n";
        }
    }

    void borrowBookByIndex(int index, string userID) {
        if (index < 1 || index > (int)books.size()) { cout << "Failed: Invalid selection.\n"; return; }
        Book* b = &books[index - 1];
        LibraryUser* u = findUser(userID);
        if (!u) { cout << "Failed: User not found.\n"; return; }
        if (!b->getAvailability()) { cout << "Failed: Book already borrowed.\n"; return; }
        u->borrowBook(b->getISBN());
        b->setAvailability(false, userID);
        saveToFiles();
        cout << "Successfully borrowed: " << b->getTitle() << " by " << b->getAuthor() << " (" << b->getISBN() << ")\n";
    }

    void returnBookByIndex(int index, string userID) {
        LibraryUser* u = findUser(userID);
        if (!u) { cout << "Failed: User not found.\n"; return; }
        vector<string> bks = u->getBorrowedBooks();
        if (bks.empty()) {
            cout << "No book's borrowed.\n";
            return;
        }
        if (index < 1 || index > (int)bks.size()) { cout << "Failed: Invalid selection.\n"; return; }
        string isbn = bks[index - 1];
        Book* b = findBook(isbn);
        if (!b) { cout << "Failed: Book not found.\n"; return; }
        u->returnBook(isbn);
        b->setAvailability(true, "");
        saveToFiles();
        cout << "Successfully returned: " << b->getTitle() << " by " << b->getAuthor() << " (" << b->getISBN() << ")\n";
    }

    void displayAllBooks() {
        cout << "\n--- Books in HayMe Awdi Library ---\n";
        if (books.empty()) { cout << "No books available.\n"; return; }
        for (int i = 0; i < (int)books.size(); i++) {
            Book &b = books[i];
            cout << (i + 1) << ". " << b.getTitle() << " by " << b.getAuthor() << " (" << b.getISBN() << ") - ";
            if (b.getAvailability()) cout << "Available";
            else {
                LibraryUser* borrower = findUser(b.getBorrowedByID());
                if (borrower) cout << "Borrowed by: " << borrower->getName() << " (" << borrower->getUserID() << ")";
                else cout << "Borrowed (Unknown User)";
            }
            cout << "\n";
        }
    }

    void displayUserBooks(string userID) {
        LibraryUser* u = findUser(userID);
        if (!u) { cout << "User not found.\n"; return; }
        vector<string> bks = u->getBorrowedBooks();
        cout << "\n--- Borrowed Books for " << u->getName() << " ---\n";
        if (bks.empty()) {

            cout << "No book's borrowed.\n";
            return;
        }
        for (int i = 0; i < (int)bks.size(); i++) {
            Book* b = findBook(bks[i]);
            if (b) cout << (i + 1) << ". " << b->getTitle() << " by " << b->getAuthor() << " (" << b->getISBN() << ")\n";
        }
    }

    void displayAllUsers() {
        cout << "\n--- Registered Users ---\n";
        if (users.empty()) { cout << "No users registered.\n"; return; }
        for (LibraryUser &u : users) {
            cout << u.getUserID() << ": " << u.getName() << " | Borrowed: ";
            vector<string> bks = u.getBorrowedBooks();

            if (bks.empty()) cout << "No book's borrowed";
            else {
                for (string &isbn : bks) {
                    Book* b = findBook(isbn);
                    if (b) cout << "[" << b->getTitle() << "] ";
                }
            }
            cout << "\n";
        }
    }

    void displayUsersNumbered() {
        cout << "\n--- Registered Users ---\n";
        if (users.empty()) { cout << "No users registered.\n"; return; }
        for (int i = 0; i < (int)users.size(); i++) {
            LibraryUser &u = users[i];
            cout << (i + 1) << ". " << u.getUserID() << ": " << u.getName() << "\n";
        }
    }

    void saveToFiles() {
        ofstream outBooks("books.txt"), outUsers("users.txt");
        for (Book &b : books) b.save(outBooks);
        for (LibraryUser &u : users) u.save(outUsers);
    }

    void loadFromFiles() {
        ifstream inBooks("books.txt");
        string line;
        while (getline(inBooks, line)) books.push_back(Book::load(line));
        ifstream inUsers("users.txt");
        while (getline(inUsers, line)) users.push_back(LibraryUser::load(line));
    }
};


void runAdminMenu(Library &library) {
    int choice;
    do {
        clearScreen();
        printBanner();
        cout << "=== Admin Menu ===\n";
        cout << "1. Books\n";
        cout << "2. Users\n";
        cout << "3. Add Book\n";
        cout << "4. Remove Book\n";
        cout << "5. Edit User\n";
        cout << "6. Remove User\n";
        cout << "7. Register User\n";
        cout << "8. Log Out\n";
        cout << "\nEnter choice: ";
        cin >> choice;

        if (choice == 1) { library.displayAllBooks(); pauseScreen(); }
        else if (choice == 2) { library.displayAllUsers(); pauseScreen(); }
        else if (choice == 3) {
            library.displayAllBooks();
            string t, a, i;
            cout << "\n--- Add New Book ---\n";
            cout << "Title: "; cin.ignore(); getline(cin, t);
            cout << "Author: "; getline(cin, a);
            cout << "ISBN: "; getline(cin, i);

            char confirm;
            cout << "\nAre you sure you want to add this book? (Y/N): ";
            cin >> confirm;
            if (confirm == 'Y' || confirm == 'y') {
                library.addBook(t, a, i);
            } else {
                cout << "Cancelled: Book not added.\n";
            }
            pauseScreen();
        } else if (choice == 4) {
            library.displayAllBooks();
            if(library.getBooksCount() > 0) {
                int num;
                cout << "\nEnter book number to remove: "; cin >> num;
                library.removeBookByIndex(num);
            }
            pauseScreen();
        } else if (choice == 5) {
            library.displayUsersNumbered();
            if (library.getUsersCount() > 0) {
                int num;
                cout << "\nEnter user number to edit: "; cin >> num;

                if (num < 1 || num > library.getUsersCount()) {
                    cout << "Failed: Invalid selection.\n";
                } else {
                    string newName;
                    cout << "\nEnter new name: "; cin.ignore(); getline(cin, newName);
                    char confirm;
                    cout << "\nAre you sure you want to update the name? (Y/N): ";
                    cin >> confirm;
                    if (confirm == 'Y' || confirm == 'y') {
                        library.editUserByIndex(num, newName);
                    } else {
                        cout << "Cancelled: User not updated.\n";
                    }
                }
            }
            pauseScreen();
        } else if (choice == 6) {
            library.displayUsersNumbered();
             if (library.getUsersCount() > 0) {
                int num;
                cout << "\nEnter user number to remove: "; cin >> num;
                library.removeUserByIndex(num);
             }
            pauseScreen();
        } else if (choice == 7) {
            string id, n;
            cout << "\n--- Register New User ---\n";
            cout << "Enter User ID: "; cin >> id;
            cout << "Enter Name: "; cin.ignore(); getline(cin, n);
            library.registerUser(id, n);
            pauseScreen();
        } else if (choice == 8) {
            char confirm;
            cout << "\nAre you sure you want to log out? (Y/N): ";
            cin >> confirm;
            if (confirm == 'Y' || confirm == 'y') {
                break;
            }
        }
    } while (true);
}


void runUserMenu(Library &library, LibraryUser* user) {
    int choice;
    do {
        clearScreen();
        printBanner();
        cout << "Welcome, " << user->getName() << " (" << user->getUserID() << ")\n";
        cout << "\n=== User Menu ===\n";
        cout << "1. Books\n";
        cout << "2. Borrow Book\n";
        cout << "3. Return Book\n";
        cout << "4. Borrowed Books\n";
        cout << "5. Log Out\n";
        cout << "\nEnter choice: ";
        cin >> choice;

        if (choice == 1) { library.displayAllBooks(); pauseScreen(); }
        else if (choice == 2) {
            library.displayAllBooks();
            if(library.getBooksCount() > 0) {
                int num;
                cout << "\nEnter book number to borrow: "; cin >> num;
                char confirm;
                cout << "\nAre you sure you want to borrow this book? (Y/N): ";
                cin >> confirm;
                if (confirm == 'Y' || confirm == 'y') {
                    library.borrowBookByIndex(num, user->getUserID());
                } else {
                    cout << "Cancelled: Book not borrowed.\n";
                }
            }
            pauseScreen();
        } else if (choice == 3) {
            library.displayUserBooks(user->getUserID());
            vector<string> bks = user->getBorrowedBooks();

            if (!bks.empty()) {
                int num;
                cout << "\nEnter borrowed book number to return: "; cin >> num;
                char confirm;
                cout << "\nAre you sure you want to return this book? (Y/N): ";
                cin >> confirm;
                if (confirm == 'Y' || confirm == 'y') {
                    library.returnBookByIndex(num, user->getUserID());
                } else {
                    cout << "Cancelled: Book not returned.\n";
                }
            }
            pauseScreen();
        } else if (choice == 4) {

            library.displayUserBooks(user->getUserID());
            pauseScreen();
        } else if (choice == 5) {
            char confirm;
            cout << "\nAre you sure you want to log out? (Y/N): ";
            cin >> confirm;
            if (confirm == 'Y' || confirm == 'y') {
                break;
            }
        }
    } while (true);
}


int main() {
    Library library;
    int role;

    while (true) {
        clearScreen();
        printBanner();
        cout << "Log in as:\n";
		cout << "1. Admin\n";
		cout << "2. User\n";
		cout << "0. Exit\n";
        cout << "\nEnter choice: ";
        cin >> role;

        if (role == 1) {
            string username, password;
            cout << "\nAdmin:\n";
            cout << "Enter username: "; cin >> username;
            cout << "Enter password: "; cin >> password;
            if (username == "Admin" && password == "00000") {
                runAdminMenu(library);
            } else {
                cout << "Invalid username or password.\n";
                pauseScreen();
            }
        } else if (role == 2) {
            string id;
            cout << "\nUser:\n";
            cout << "Enter User ID: "; cin >> id;
            LibraryUser* user = library.findUser(id);
            if (user) {
                runUserMenu(library, user);
            } else {
                cout << "User ID not found. Please contact HayMe Awdi to register.\n";
                pauseScreen();
            }
        } else if (role == 0) break;
    }

    library.saveToFiles();
    cout << "Thank you for using the HayMe Awdi Library Management System!\n";
    return 0;
}
