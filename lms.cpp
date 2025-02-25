#include <bits/stdc++.h>
using namespace std;

vector<vector<string>> content;

// File handling functions
void readfile(string fname);
void writefile(vector<vector<string>> par, string fname);
void writefileappend(vector<string> par, string fname);



// Base class: User
class User {
    protected:
        string password;
    public:
        string name;
        string id;
        int role;

        virtual void display_menu() = 0; // Pure virtual function
        void login();
        void see_all_books();
        void see_issued_books();
        virtual void issue_book(string bookname) = 0;
        void return_book(string isbncode);
        int calc_fine();
        void pay_fine();
};

// Derived class: Student
class Student : public User {
    public:
        void display_menu() override;
        void issue_book(string bookname) override;
};

// Derived class: Faculty
class Faculty : public User {
    public:
        void display_menu() override;
        void issue_book(string bookname) override;
};

// Derived class: Librarian
class Librarian : public User {
    public:
        void display_menu() override;
        void issue_book(string bookname) override;
        void add_book();
        void remove_book();
        void add_user();
        void remove_user();
};

void User::see_all_books() {
    readfile("all_books_data.csv");
    cout << "Title | Author | Publisher | ISBN | Issued\n";
    for (auto &book : content) {
        for (auto &field : book) cout << field << " | ";
        cout << "\n";
    }
}

void User::see_issued_books() {
    readfile("issued_books_data.csv");
    bool hasBooks = false;
    for (auto &entry : content) {
        if (entry[0] == id) {
            hasBooks = true;
            cout << "Issued Books: ";
            for (size_t i = 1; i < entry.size(); i++) cout << entry[i] << " ";
            cout << "\n";
        }
    }
    if (!hasBooks) cout << "No books issued.\n";
}

void User::return_book(string isbncode) {
    readfile("issued_books_data.csv");
    
    bool found = false;
    for (auto &entry : content) {
        if (entry[0] == id && entry[2] == isbncode) {
            content.erase(remove(content.begin(), content.end(), entry), content.end());
            found = true;
            break;
        }
    }

    if (found) {
        writefile(content, "issued_books_data.csv");
        cout << "Book returned successfully.\n";

        // Update book status in all_books_data.csv
        readfile("all_books_data.csv");
        for (auto &book : content) {
            if (book[3] == isbncode) {
                book[4] = "0"; // Mark as available
                break;
            }
        }
        writefile(content, "all_books_data.csv");
    } else {
        cout << "You have not issued this book.\n";
    }
}

void User::login() {
    cout << "Enter your ID: ";
    cin >> id;
    cout << "Enter your Password: ";
    cin >> password;
    
    readfile("all_users_data.csv");

    for (auto &user : content) {
        if (user.size() < 4) continue;

        if (user[1] == id && user[2] == password) {  
            name = user[0];
            role = stoi(user[3]); // Store role
            cout << "Login successful! Welcome " << name << ".\n";
            return;
        }
    }
    cout << "Invalid credentials. Exiting...\n";
    exit(1);
}

int User::calc_fine() {
    readfile("issued_books_data.csv");
    int fine = 0;
    time_t current_time = time(0);
    for (auto &entry : content) {
        if (entry[0] == id) {
            time_t issue_time = stoi(entry[3]);
            int days_borrowed = (current_time - issue_time) / 86400;
            int allowed_days = (role == 1) ? 15 : 30;
            if (days_borrowed > allowed_days) {
                fine += (days_borrowed - allowed_days) * 10;
            }
        }
    }
    return fine;
}

void User::pay_fine() {
    int fine = calc_fine();
    if (fine > 0) {
        cout << "Your total fine is: " << fine << " rupees.\n";
        cout << "Simulating payment...\n";
        // Reset fine
        cout << "Payment successful. Your fine has been cleared.\n";
    } else {
        cout << "No fines to pay.\n";
    }
}

void Student::display_menu() {
    while (true) {
        cout << "\nStudent Menu:\n1. See All Books\n2. See Issued Books\n3. Issue Book\n4. Return Book\n5. Pay Fine\n6. Logout\nEnter choice: ";
        int choice;
        cin >> choice;

        if (choice == 1) see_all_books();
        else if (choice == 2) see_issued_books();
        else if (choice == 3) {
            string bookname;
            cout << "Enter book name: ";
            cin >> ws;  
            getline(cin, bookname);
            issue_book(bookname);
        } 
        else if (choice == 4) {
            string isbn;
            cout << "Enter ISBN of book to return: ";
            cin >> isbn;
            return_book(isbn);
        } 
        else if (choice == 5) {
            pay_fine();
        }
        else if (choice == 6) {
            cout << "Logging out...\n";
            break;
        } 
        else {
            cout << "Invalid choice. Please try again.\n";
        }
    }
}

void Student::issue_book(string bookname) {
    if (calc_fine() > 0) {
        cout << "You have unpaid fines. Please clear them before borrowing.\n";
        return;
    }
    readfile("all_books_data.csv");
    for (auto &book : content) {
        if (book[0] == bookname && book[4] == "0") {
            book[4] = "1";
            writefile(content, "all_books_data.csv");
            writefileappend({id, bookname, book[3], to_string(time(0))}, "issued_books_data.csv");
            cout << "Book issued successfully.\n";
            return;
        }
    }
    cout << "Book not available.\n";
}

void Faculty::display_menu() {
    while (true) {
        cout << "\nFaculty Menu:\n1. See All Books\n2. See Issued Books\n3. Issue Book\n4. Return Book\n5. Pay Fine\n6. Logout\nEnter choice: ";
        int choice;
        cin >> choice;

        if (choice == 1) see_all_books();
        else if (choice == 2) see_issued_books();
        else if (choice == 3) {
            string bookname;
            cout << "Enter book name: ";
            cin >> ws;
            getline(cin, bookname);
            issue_book(bookname);
        }
        else if (choice == 4) {
            string isbn;
            cout << "Enter ISBN of book to return: ";
            cin >> isbn;
            return_book(isbn);
        }
        else if (choice == 5) {
            pay_fine();
        }
        else if (choice == 6) {
            cout << "Logging out...\n";
            break;
        }
        else {
            cout << "Invalid choice. Please try again.\n";
        }
    }
}

void Faculty::issue_book(string bookname) {
    readfile("issued_books_data.csv");
    time_t current_time = time(0);
    int book_count = 0;

    for (auto &entry : content) {
        if (entry[0] == id) {
            time_t issue_time = stoi(entry[3]);
            int days_borrowed = (current_time - issue_time) / 86400;
            if (days_borrowed > 60) {
                cout << "You have a book overdue by more than 60 days. You cannot borrow a new book.\n";
                return;
            }
            book_count++;
        }
    }

    if (book_count >= 5) {
        cout << "You have already borrowed the maximum allowed books (5). Return a book to borrow another.\n";
        return;
    }

    readfile("all_books_data.csv");
    bool book_found = false;

    for (auto &book : content) {
        if (strcasecmp(book[0].c_str(), bookname.c_str()) == 0) {
            book_found = true;

            if (book[4] == "0") {
                book[4] = "1";
                writefile(content, "all_books_data.csv");
                writefileappend({id, book[0], book[3], to_string(current_time)}, "issued_books_data.csv");
                cout << "Book issued successfully.\n";
                return;
            } else {
                cout << "Book is already issued.\n";
                return;
            }
        }
    }

    if (!book_found) {
        cout << "Book not found.\n";
    }
}

void Librarian::display_menu() {
    while (true) {
        cout << "\nLibrarian Menu:\n1. Add Book\n2. Remove Book\n3. Add User\n4. Remove User\n5. Logout\nEnter choice: ";
        int choice;
        cin >> choice;

        if (choice == 1) add_book();
        else if (choice == 2) remove_book();
        else if (choice == 3) add_user();
        else if (choice == 4) remove_user();
        else if (choice == 5) {
            cout << "Logging out...\n";
            break;
        }
        else {
            cout << "Invalid choice. Please try again.\n";
        }
    }
}

void Librarian::issue_book(string bookname) {
    cout << "Librarians cannot issue books.\n";
}

void Librarian::add_book() {
    string title, author, publisher, isbn;
    cout << "Enter book title: ";
    cin >> ws;
    getline(cin, title);
    cout << "Enter book author: ";
    getline(cin, author);
    cout << "Enter book publisher: ";
    getline(cin, publisher);
    cout << "Enter book ISBN: ";
    getline(cin, isbn);

    writefileappend({title, author, publisher, isbn, "0"}, "all_books_data.csv");
    cout << "Book added successfully.\n";
}

void Librarian::remove_book() {
    string isbn;
    cout << "Enter ISBN of book to remove: ";
    cin >> isbn;

    readfile("all_books_data.csv");
    bool found = false;
    for (auto &book : content) {
        if (book[3] == isbn) {
            content.erase(remove(content.begin(), content.end(), book), content.end());
            found = true;
            break;
        }
    }

    if (found) {
        writefile(content, "all_books_data.csv");
        cout << "Book removed successfully.\n";
    } else {
        cout << "Book not found.\n";
    }
}

void Librarian::add_user() {
    string name, id, password;
    int role;
    cout << "Enter user name: ";
    cin >> ws;
    getline(cin, name);
    cout << "Enter user ID: ";
    getline(cin, id);
    cout << "Enter user password: ";
    getline(cin, password);
    cout << "Enter user role (1 for Student, 2 for Faculty, 3 for Librarian): ";
    cin >> role;

    writefileappend({name, id, password, to_string(role)}, "all_users_data.csv");
    cout << "User added successfully.\n";
}

void Librarian::remove_user() {
    string id;
    cout << "Enter ID of user to remove: ";
    cin >> id;

    readfile("all_users_data.csv");
    bool found = false;
    for (auto &user : content) {
        if (user[1] == id) {
            content.erase(remove(content.begin(), content.end(), user), content.end());
            found = true;
            break;
        }
    }

    if (found) {
        writefile(content, "all_users_data.csv");
        cout << "User removed successfully.\n";
    } else {
        cout << "User not found.\n";
    }
}

class Book {
    public:
        string title;
        string author;
        string isbn;
        bool isIssued;
    
        Book(string t, string a, string i, bool issued = false)
            : title(t), author(a), isbn(i), isIssued(issued) {}
    
        string toCSV() const {
            return title + "," + author + "," + isbn + "," + (isIssued ? "1" : "0");
        }
    };
    
class Library {
    private:
        vector<Book> books;
    
    public:
        void loadBooksFromCSV(const string& filename) {
            ifstream file(filename);
            if (!file.is_open()) {
                cerr << "Error opening file: " << filename << endl;
                return;
            }
    
            string line;
            while (getline(file, line)) {
                stringstream ss(line);
                string title, author, isbn, issuedStr;
                getline(ss, title, ',');
                getline(ss, author, ',');
                getline(ss, isbn, ',');
                getline(ss, issuedStr, ',');
    
                bool isIssued = (issuedStr == "1");
                books.emplace_back(title, author, isbn, isIssued);
            }
    
            file.close();
        }
    
        void saveBooksToCSV(const string& filename) {
            ofstream file(filename);
            if (!file.is_open()) {
                cerr << "Error opening file for writing: " << filename << endl;
                return;
            }
    
            for (const auto& book : books) {
                file << book.toCSV() << endl;
            }
    
            file.close();
        }
    
        void addBook(const Book& book) {
            books.push_back(book);
        }
    
        void displayBooks() {
            cout << "\nAvailable Books in the Library:" << endl;
            for (const auto& book : books) {
                cout << "Title: " << book.title
                     << ", Author: " << book.author
                     << ", ISBN: " << book.isbn
                     << ", Issued: " << (book.isIssued ? "Yes" : "No")
                     << endl;
            }
        }
    };

void readfile(string fname) {
    content.clear();
    vector<string> row;
    string line, word;
    fstream file(fname, ios::in);
    if (file.is_open()) {
        while (getline(file, line)) {
            row.clear();
            stringstream str(line);
            while (getline(str, word, ',')) row.push_back(word);
            content.push_back(row);
        }
        file.close();
    } else cout << "Could not open the file: " << fname << "\n";
}

void writefile(vector<vector<string>> par, string fname) {  
    fstream fout(fname, ios::out);
    for (auto &x : par) {
        for (size_t i = 0; i < x.size(); i++) {
            fout << x[i];
            if (i < x.size() - 1) fout << ",";
        }
        fout << "\n";
    }
    fout.close();
}

void writefileappend(vector<string> par, string fname) {  
    fstream fout(fname, ios::out | ios::app);
    for (size_t i = 0; i < par.size(); i++) {
        fout << par[i];
        if (i < par.size() - 1) fout << ",";
    }
    fout << "\n";
    fout.close();
}

int main() {
    Library library;
    library.loadBooksFromCSV("all_books_data.csv");
    string userType;
    cout << "Enter user type (Student/Faculty/Librarian): ";
    cin >> userType;

    User *user = nullptr;
    if (userType == "Student") user = new Student();
    else if (userType == "Faculty") user = new Faculty();
    else if (userType == "Librarian") user = new Librarian();
    else {
        cout << "Invalid user type. Exiting...\n";
        return 1;
    }

    user->login();
    user->display_menu();

    delete user;
    return 0;
}