#include <bits/stdc++.h>
using namespace std;

vector<vector<string>> content;

// File handling functions
void readfile(const string &fname);
void writefile(const vector<vector<string>> &par, const string &fname);
void writefileappend(const vector<string> &par, const string &fname);

// Case-insensitive comparison
string toLower(string str)
{
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

bool caseInsensitiveCompare(const string &str1, const string &str2)
{
    return toLower(str1) == toLower(str2);
}

// Base class: User
class User
{
protected:
    string password;
    Account account; // For fine tracking

public:
    string name;
    string id;
    int role;

    // Constructor
    User(string id, string name, int role) : id(id), name(name), role(role) {}

    // Getters
    string getId() const { return id; }
    string getName() const { return name; }
    int getRole() const { return role; }

    // CSV Representation
    string toCSV() const
    {
        return id + "," + name + "," + to_string(role);
    }

    // Factory Method: Return a Derived Class (Student, Faculty, Librarian)
    static User *fromCSV(const string &line)
    {
        stringstream ss(line);
        string id, name, roleStr;
        getline(ss, id, ',');
        getline(ss, name, ',');
        getline(ss, roleStr, ',');

        int role = stoi(roleStr);

        // Return appropriate object based on role
        if (role == 1)
            return new Student(id, name, role);
        else if (role == 2)
            return new Faculty(id, name, role);
        else if (role == 3)
            return new Librarian(id, name, role);
        else
            throw invalid_argument("Invalid user role in CSV.");
    }

    // Pure virtual functions for polymorphism
    virtual void login() = 0;
    virtual void display_menu() = 0;
    virtual void issue_book(const string &bookname) = 0;

    // Virtual destructor for safe polymorphic deletion
    virtual ~User() {}

    // void see_borrowing_history();

    virtual void issue_book(const string &bookname) = 0;

    void see_all_books()
    {
        readfile("all_books_data.csv");
        cout << "Title | Author | Publisher | ISBN | Issued\n";
        for (auto &book : content)
        {
            for (auto &field : book)
                cout << field << " | ";
            cout << "\n";
        }
    }
    
    void see_issued_books()
    {
        account.showBorrowedBooks();
    }
    
    void return_book(const string &isbncode)
    {
        if (account.removeBorrowedBook(isbncode))
        {
            saveAccountData();
            cout << "Book returned successfully.\n";
        }
        else
        {
            cout << "You have not issued this book.\n";
        }
    } 
    
    void return_book(const string &isbncode);

    // void pay_fine();

    // void saveAccountData();

    void saveAccountData()
    {
        readfile("issued_books_data.csv");

        // Remove existing records for the user
        content.erase(remove_if(content.begin(), content.end(), [&](const vector<string> &entry)
                                { return entry[0] == id; }),
                      content.end());

        // Add updated borrowed books (time_t is already in seconds)
        for (const auto &book : account.getBorrowedBooks())
        {
            content.push_back({id, name, book.first, to_string(book.second)});
        }

        writefile(content, "issued_books_data.csv");
    }


    void User::pay_fine()
{
    if (account.calculateFine() > 0)
    {
        cout << "Your total fine is: " << account.getFineAmount() << " rupees.\n";
        cout << "Simulating payment...\n";
        account.clearFine();
        saveAccountData();
        cout << "Payment successful. Fine cleared.\n";
    }
    else
    {
        cout << "No fines to pay.\n";
    }
}

// Returns User ID (for consistency in Library functions)
string getUserId() const { return id; }

// Check if user has borrowed a specific book (Delegates to Account)
bool hasBorrowed(const string &isbn) const {
    return account.hasBorrowed(isbn);
}

// Access to Account object (if needed externally)
Account &getAccount() { return account; }
const Account &getAccount() const { return account; }

};

class Account
{
private:
    vector<pair<string, time_t>> borrowedBooks; // Stores ISBN and issue date
    int fineAmount;
    int maxBorrowLimit;
    int borrowPeriod;
    bool isFaculty;

    // Helper function to calculate days between two time_t values
    int calculateDays(time_t now, time_t past) const
    {
        return (now - past) / 86400; // 86400 seconds = 1 day
    }

public:
    // Constructor
    Account(bool faculty = false) : fineAmount(0), isFaculty(faculty)
    {
        maxBorrowLimit = faculty ? 5 : 3;
        borrowPeriod = faculty ? 30 : 15;
    }

    void pay_fine();

    // Add borrowed book
    bool addBorrowedBook(const string &isbn)
    {
        if ((int)borrowedBooks.size() >= maxBorrowLimit)
            return false;

        borrowedBooks.emplace_back(isbn, time(0));
        calculateFine(); // Ensure fine is updated immediately
        return true;
    }

    // Remove returned book
    bool removeBorrowedBook(const string &isbn)
    {
        auto it = find_if(borrowedBooks.begin(), borrowedBooks.end(), [&](const pair<string, time_t> &book)
                          { return book.first == isbn; });

        if (it != borrowedBooks.end())
        {
            borrowedBooks.erase(it);
            calculateFine(); // Ensure fine is updated immediately
            return true;
        }
        return false;
    }

    int calculateFine() const
    {
        // No fines for faculty
        if (isFaculty)
            return 0;
    
        // Use a local variable to store the calculated fine
        int totalFine = 0;
    
        // Get current time
        time_t currentTime = time(0);
    
        // Iterate through borrowed books to calculate fines
        for (const auto &book : borrowedBooks)
        {
            int daysBorrowed = calculateDays(currentTime, book.second);
    
            // If overdue, calculate the fine
            if (daysBorrowed > borrowPeriod)
            {
                totalFine += (daysBorrowed - borrowPeriod) * 10; // ₹10 per overdue day
            }
        }
    
        return totalFine; // Return the calculated fine
    }
        

// Clear fine only if there is no outstanding amount
void clearFine()
{
    if (calculateFine() == 0)
    {
        fineAmount = 0;
        cout << "Fine cleared successfully!" << endl;
    }
    else
    {
        cout << "Cannot clear fine—outstanding balance exists." << endl;
    }
}


bool canBorrow() const
{
    int currentFine = calculateFine(); // Avoid redundant fine calculation

    return (int)borrowedBooks.size() < maxBorrowLimit &&
           currentFine == 0 &&                  // Ensure real-time fine is 0
           (!isFaculty || !hasOverdue60Days()); // Faculty: No 60+ days overdue
}



bool hasOverdue60Days() const
{
    if (!isFaculty) return false; // Only faculty have this restriction

    time_t currentTime = time(0);

    for (const auto &book : borrowedBooks)
    {
        int daysBorrowed = calculateDays(currentTime, book.second);

        // If any book is overdue by more than 60 days, return true immediately
        if (daysBorrowed > 60)
            return true;
    }

    return false;
}

// Display borrowed books with overdue status
void showBorrowedBooks() const
{
    if (borrowedBooks.empty())
    {
        cout << "No books currently borrowed.\n";
        return;
    }

    cout << "Borrowed Books (ISBN | Days Borrowed | Status):\n";
    time_t now = time(0);
    for (const auto &book : borrowedBooks)
    {
        int daysBorrowed = calculateDays(now, book.second);
        cout << book.first << " | " << daysBorrowed << " days";

        if (daysBorrowed > borrowPeriod) cout << " | OVERDUE";
        cout << "\n";
    }
}

// Check if a specific book is borrowed
bool hasBorrowed(const string &isbn) const {
    return any_of(borrowedBooks.begin(), borrowedBooks.end(),
                  [&](const pair<string, time_t> &book) { return book.first == isbn; });
}

    int getFineAmount() const { return fineAmount; }
    int getBorrowedCount() const { return (int)borrowedBooks.size(); }
    int getMaxBorrowLimit() const { return maxBorrowLimit; }
    vector<pair<string, time_t>> getBorrowedBooks() const { return borrowedBooks; }
};

// Derived class: Student
class Student : public User
{
public:

    // Default constructor (for main())
    Student() : User("DefaultID", "DefaultName", 1) {}

    // Parameterized constructor (for fromCSV())
    Student(const string &id, const string &name, int role) : User(id, name, role) {}

    void login()
    {
        cout << "Student login\n";
    }
    void display_menu()
    {
        while (true)
        {
            cout << "\nStudent Menu:\n"
                 << "1. See All Books\n"
                 << "2. See Issued Books\n"
                 << "3. Issue Book\n"
                 << "4. Return Book\n"
                 << "5. Pay Fine\n"
                 << "6. Logout\n"
                 << "Enter choice: ";
            
            int choice;
            cin >> choice;
    
            if (cin.fail()) // Handle non-integer input
            {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid input. Please enter a number.\n";
                continue;
            }
    
            switch (choice)
            {
            case 1:
                see_all_books();
                break;
            case 2:
                see_issued_books();
                break;
            case 3:
            {
                string bookname;
                cout << "Enter book name: ";
                cin >> ws; // Clear whitespace before getline()
                getline(cin, bookname);
                issue_book(bookname);
                break;
            }
            case 4:
            {
                string isbn;
                cout << "Enter ISBN of book to return: ";
                cin >> isbn;
                return_book(isbn);
                break;
            }
            case 5:
                pay_fine();
                break;
            case 6:
                cout << "Logging out...\n";
                return; // Exit the loop and function
            default:
                cout << "Invalid choice. Please try again.\n";
            }
        }
    }
    
    void issue_book(const string &bookname)
    {
        // Count borrowed books for the current user
        readfile("issued_books_data.csv");
        int book_count = 0;
        for (auto &entry : content)
        {
            if (entry[0] == id)
            {
                book_count++;
                if (entry[1] == bookname)
                {
                    cout << "You have already borrowed this book.\n";
                    return;
                }
            }
        }
    
        if (book_count >= 3)
        {
            cout << "You have already borrowed the maximum allowed books (3). Return a book to borrow another.\n";
            return;
        }
    
        if (account.calculateFine() > 0)
        {
            cout << "You have unpaid fines. Please clear them before borrowing.\n";
            return;
        }
    
        // Check availability in "all_books_data.csv"
        readfile("all_books_data.csv");
        bool book_found = false;
        for (auto &book : content)
        {
            if (book[0] == bookname && book[4] == "0") // Book name + availability check
            {
                book[4] = "1"; // Mark as issued
                book_found = true;
                break;
            }
        }
    
        if (!book_found)
        {
            cout << "Book not available or does not exist.\n";
            return;
        }
    
        // Save updated book availability
        writefile(content, "all_books_data.csv");
    
        // Log issued book for the student
        writefileappend({id, bookname, to_string(time(0))}, "issued_books_data.csv");
        cout << "Book issued successfully: " << bookname << "\n";
    }
    };

// Derived class: Faculty
class Faculty : public User
{
public:

    Faculty() : User("DefaultID", "DefaultName", 2) {}

    Faculty(const string &id, const string &name, int role) : User(id, name, role) {}

    void login() override
    {
        cout << "Faculty login\n";
    }
    void display_menu() override
    {
        cout << "Faculty Menu\n";
    }
    void issue_book(const string &bookname) override;
};

// Derived class: Librarian
class Librarian : public User
{
public:

    Librarian() : User("DefaultID", "DefaultName", 3) {}
    
    Librarian(const string &id, const string &name, int role) : User(id, name, role) {}


    void login() override
    {
        cout << "Librarian login\n";
    }
    void display_menu() override
    {
        cout << "Librarian Menu\n";
    }
    void issue_book(const string &bookname) override { cout << "Librarians cannot issue books.\n"; }
    void add_book();
    void remove_book();
    void update_book();
    void add_user();
    void remove_user();
    void update_user();
};


void Faculty::display_menu()
{
    while (true)
    {
        cout << "\nFaculty Menu:\n1. See All Books\n2. See Issued Books\n3. Issue Book\n4. Return Book\n5. Pay Fine\n6. Logout\nEnter choice: ";
        int choice;
        cin >> choice;

        if (choice == 1)
            see_all_books();
        else if (choice == 2)
            see_issued_books();
        else if (choice == 3)
        {
            string bookname;
            cout << "Enter book name: ";
            cin >> ws;
            getline(cin, bookname);
            issue_book(bookname);
        }
        else if (choice == 4)
        {
            string isbn;
            cout << "Enter ISBN of book to return: ";
            cin >> isbn;
            return_book(isbn);
        }
        else if (choice == 5)
        {
            pay_fine();
        }
        else if (choice == 6)
        {
            cout << "Logging out...\n";
            break;
        }
        else
        {
            cout << "Invalid choice. Please try again.\n";
        }
    }
}

void Faculty::issue_book(const string &bookname)
{
    readfile("issued_books_data.csv");
    time_t current_time = time(0);
    int book_count = 0;

    for (auto &entry : content)
    {
        if (entry[0] == id)
        {
            time_t issue_time = stoi(entry[3]);
            int days_borrowed = (current_time - issue_time) / 86400;
            if (days_borrowed > 60)
            {
                cout << "You have a book overdue by more than 60 days. You cannot borrow a new book.\n";
                return;
            }
            book_count++;
        }
    }

    if (book_count >= 5)
    {
        cout << "You have already borrowed the maximum allowed books (5). Return a book to borrow another.\n";
        return;
    }

    readfile("all_books_data.csv");
    bool book_found = false;

    for (auto &book : content)
    {
        if (strcasecmp(book[0].c_str(), bookname.c_str()) == 0)
        {
            book_found = true;

            if (book[4] == "0")
            {
                book[4] = "1";
                writefile(content, "all_books_data.csv");
                writefileappend({id, book[0], book[3], to_string(current_time)}, "issued_books_data.csv");
                cout << "Book issued successfully.\n";
                return;
            }
            else
            {
                cout << "Book is already issued.\n";
                return;
            }
        }
    }

    if (!book_found)
    {
        cout << "Book not found.\n";
    }
}

void Librarian::display_menu()
{
    while (true)
    {
        cout << "\nLibrarian Menu:\n1. Add Book\n2. Remove Book\n3. Add User\n4. Remove User\n5. Logout\nEnter choice: ";
        int choice;
        cin >> choice;

        if (choice == 1)
            add_book();
        else if (choice == 2)
            remove_book();
        else if (choice == 3)
            add_user();
        else if (choice == 4)
            remove_user();
        else if (choice == 5)
        {
            cout << "Logging out...\n";
            break;
        }
        else
        {
            cout << "Invalid choice. Please try again.\n";
        }
    }
}

void Librarian::issue_book(const string &bookname)
{
    cout << "Librarians cannot issue books.\n";
}

void Librarian::add_book()
{
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

void Librarian::remove_book()
{
    string isbn;
    cout << "Enter ISBN of book to remove: ";
    cin >> isbn;

    readfile("all_books_data.csv");
    bool found = false;
    for (auto &book : content)
    {
        if (book[3] == isbn)
        {
            content.erase(remove(content.begin(), content.end(), book), content.end());
            found = true;
            break;
        }
    }

    if (found)
    {
        writefile(content, "all_books_data.csv");
        cout << "Book removed successfully.\n";
    }
    else
    {
        cout << "Book not found.\n";
    }
}

void Librarian::add_user()
{
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

void Librarian::remove_user()
{
    string id;
    cout << "Enter ID of user to remove: ";
    cin >> id;

    readfile("all_users_data.csv");
    bool found = false;
    for (auto &user : content)
    {
        if (user[1] == id)
        {
            content.erase(remove(content.begin(), content.end(), user), content.end());
            found = true;
            break;
        }
    }

    if (found)
    {
        writefile(content, "all_users_data.csv");
        cout << "User removed successfully.\n";
    }
    else
    {
        cout << "User not found.\n";
    }
}

void Librarian::update_book()
{
    string isbn;
    cout << "Enter ISBN of the book to update: ";
    cin >> ws;
    getline(cin, isbn);

    readfile("all_books_data.csv");

    bool found = false;
    for (auto &book : content)
    {
        if (book[3] == isbn)
        {
            found = true;

            cout << "Enter new title: ";
            getline(cin, book[0]);
            cout << "Enter new author: ";
            getline(cin, book[1]);
            cout << "Enter new publisher: ";
            getline(cin, book[2]);

            int year;
            cout << "Enter new year: ";
            cin >> year;
            book[4] = to_string(year);

            writefile(content, "all_books_data.csv");
            cout << "Book updated successfully.\n";
            return;
        }
    }

    if (!found)
    {
        cout << "Error: Book not found.\n";
    }
}

void Librarian::update_user()
{
    string user_id;
    cout << "Enter user ID to update: ";
    cin >> user_id;

    readfile("all_users_data.csv");
    bool found = false;

    for (auto &user : content)
    {
        if (user[1] == user_id)
        {
            found = true;
            cout << "Updating details for: " << user[0] << "\n";

            cout << "Enter new name: ";
            cin >> ws;
            getline(cin, user[0]);

            cout << "Enter new password: ";
            getline(cin, user[2]);

            writefile(content, "all_users_data.csv");
            cout << "User updated successfully.\n";
            return;
        }
    }

    if (!found)
    {
        cout << "User not found.\n";
    }
}

void Librarian::add_book()
{
    string title, author, publisher, isbn;
    int year;

    cout << "Enter book title: ";
    cin >> ws;
    getline(cin, title);
    cout << "Enter author: ";
    getline(cin, author);
    cout << "Enter publisher: ";
    getline(cin, publisher);
    cout << "Enter year: ";
    cin >> year;
    cout << "Enter ISBN: ";
    cin >> ws;
    getline(cin, isbn);

    readfile("all_books_data.csv");

    for (const auto &book : content)
    {
        if (book[3] == isbn)
        {
            cout << "Error: ISBN already exists.\n";
            return;
        }
    }

    writefileappend({title, author, publisher, isbn, to_string(year), "0"}, "all_books_data.csv");
    cout << "Book added successfully.\n";
}

class Book
{
private:
    string isbn;
    string title;
    string author;
    int year;
    bool isIssued;

public:
    Book(string isbn, string title, string author, int year, bool isIssued = false)
        : isbn(isbn), title(title), author(author), year(year), isIssued(isIssued) {}

    void setIsbn(const string &newIsbn) {
        isbn = newIsbn;
    }

    // Getters
    string getISBN() const { return isbn; }
    string getTitle() const { return title; }
    string getAuthor() const { return author; }
    int getYear() const { return year; }
    bool getIsIssued() const { return isIssued; }

    // Setters
    void setTitle(const string &newTitle) { title = newTitle; }
    void setAuthor(const string &newAuthor) { author = newAuthor; }
    void setIsbn(const string &newIsbn) { isbn = newIsbn; }
    void setYear(int newYear) { year = newYear; }
    void setIsIssued(bool status) { isIssued = status; }

    string toCSV() const
    {
        return isbn + "," + title + "," + author + "," + to_string(year) + "," + (isIssued ? "1" : "0");
    }

    static Book fromCSV(const string &line)
    {
        stringstream ss(line);
        string isbn, title, author, yearStr, isIssuedStr;

        getline(ss, isbn, ',');
        getline(ss, title, ',');
        getline(ss, author, ',');
        getline(ss, yearStr, ',');
        getline(ss, isIssuedStr, ',');

        return Book(isbn, title, author, stoi(yearStr), isIssuedStr == "1");
    }
};

class Library
{
private:
    vector<Book> books;
    vector<User*> users;

    void loadBooks()
    {
        ifstream file("all_books_data.csv");
        string line;
        while (getline(file, line))
        {
            books.push_back(Book::fromCSV(line));
        }
        file.close();

        if (books.empty())
        {
            initializeDefaultBooks();
            saveBooks();
        }
    }

    void saveBooks()
    {
        ofstream file("all_books_data.csv");
        for (const auto &book : books)
        {
            file << book.toCSV() << "\n";
        }
        file.close();
    }

    void loadUsers()
    {
        ifstream file("all_users_data.csv");
        string line;
        while (getline(file, line))
        {
            users.push_back(User::fromCSV(line));
        }
        file.close();


        if (users.empty())
        {
            cerr << "Error: User database is empty. Exiting the system...\n";
            exit(1); // Terminates the program
        }
    }

    void saveUsers()
    {
        ofstream file("all_users_data.csv");
        for (const auto &user : users) // user is a User* (pointer to User)
        {
            file << user->toCSV() << "\n"; // Use '->' to call method on a pointer
        }
        file.close();
    }
    

    void logReturn(const Book &book)
    {
        ofstream historyFile("borrow_history.csv", ios::app);
        time_t now = time(0);
        tm *ltm = localtime(&now);
        historyFile << book.getISBN() << "," << book.getTitle() << "," << book.getAuthor() << "," << book.getYear() << ","
                    << (1900 + ltm->tm_year) << "-" << (1 + ltm->tm_mon) << "-" << ltm->tm_mday << "\n";
        historyFile.close();
    }

    bool Library::isIsbnDuplicate(const string &isbn) {
        for (const auto &book : books) {
            if (book.getISBN() == isbn) {
                return true; // Duplicate found
            }
        }
        return false;
    }

    void initializeDefaultBooks()
    {
        vector<Book> defaultBooks = {
            Book("978-0131103627", "The C++ Programming Language", "Bjarne Stroustrup", 2013),
            Book("978-0201616224", "Design Patterns", "Erich Gamma", 1994),
            Book("978-0262033848", "Introduction to Algorithms", "Thomas H. Cormen", 2009),
            Book("978-0132350884", "Clean Code", "Robert C. Martin", 2008),
            Book("978-0201633610", "The Mythical Man-Month", "Frederick P. Brooks Jr.", 1995),
            Book("978-0131101630", "The Art of Computer Programming", "Donald E. Knuth", 2011),
            Book("978-0596007126", "Head First Design Patterns", "Eric Freeman", 2004),
            Book("978-1449355739", "Python Cookbook", "David Beazley", 2013),
            Book("978-0134494166", "Effective Modern C++", "Scott Meyers", 2014),
            Book("978-1491950357", "Learning Python", "Mark Lutz", 2013)};

        books.insert(books.end(), defaultBooks.begin(), defaultBooks.end());
    }
    

public:
    Library()
    {
        loadBooks();
        loadUsers();
    }

    void displayBooks() const
    {
        for (const auto &book : books)
        {
            cout << "ISBN: " << book.getISBN() << ", Title: " << book.getTitle()
                 << ", Author: " << book.getAuthor() << ", Year: " << book.getYear()
                 << ", Issued: " << (book.getIsIssued() ? "Yes" : "No") << endl;
        }
    }

    void Library::addBook() {
        string title, author, isbn, year;
        cout << "Enter book title: ";
        cin.ignore();
        getline(cin, title);
        cout << "Enter author: ";
        getline(cin, author);
        cout << "Enter ISBN: ";
        getline(cin, isbn);
        cout << "Enter publication year: ";
        getline(cin, year);
    
        // Check if ISBN already exists
        if (isIsbnDuplicate(isbn)) {
            cout << "Error: A book with ISBN " << isbn << " already exists!" << endl;
            return;
        }
    
        Book newBook(isbn, title, author, stoi(year), true);
        books.push_back(newBook);

        saveBooks();
        cout << "Book added successfully!" << endl;
    }    

    void Library::updateBook() {
        string isbn;
        cout << "Enter ISBN of the book to update: ";
        cin >> isbn;
    
        // Find the book by ISBN
        auto it = find_if(books.begin(), books.end(), [&](const Book &book) {
            return book.getISBN() == isbn;
        });
    
        if (it == books.end()) {
            cout << "Error: Book with ISBN " << isbn << " not found!" << endl;
            return;
        }
    
        cout << "Updating book details (Leave blank to keep current values):\n";
    
        string newTitle, newAuthor, newIsbn, newYear;
    
        cout << "Enter new title: ";
        cin.ignore();
        getline(cin, newTitle);
        if (!newTitle.empty()) it->setTitle(newTitle);
    
        cout << "Enter new author: ";
        getline(cin, newAuthor);
        if (!newAuthor.empty()) it->setAuthor(newAuthor);
    
        cout << "Enter new ISBN: ";
        getline(cin, newIsbn);
    
        // Check for ISBN duplication (if new ISBN is provided)
        if (!newIsbn.empty() && newIsbn != isbn && isIsbnDuplicate(newIsbn)) {
            cout << "Error: A book with ISBN " << newIsbn << " already exists!" << endl;
            return;
        }
        if (!newIsbn.empty()) it->setIsbn(newIsbn);
    
        cout << "Enter new publication year: ";
        getline(cin, newYear);
        if (!newYear.empty()) {
            it->setYear(stoi(newYear));
        }
        if (!newYear.empty() && all_of(newYear.begin(), newYear.end(), ::isdigit)) {
            it->setYear(stoi(newYear));
        } else {
            cout << "Invalid year format!" << endl;
        }
                
    
        saveBooks();
        cout << "Book updated successfully!" << endl;
    }    

    void Library::returnBook(const string &userId, const string &isbn) {
        auto userIt = find_if(users.begin(), users.end(),
                              [&](const User *user) { return user->getUserId() == userId; });
    
        if (userIt == users.end()) {
            cout << "Error: User not found!" << endl;
            return;
        }
    
        User *user = *userIt;
    
        // Check if the book is borrowed
        if (!user->hasBorrowed(isbn)) {
            cout << "Error: This book is not borrowed by the user!" << endl;
            return;
        }
    
        // Return the book
        if (user->getAccount().removeBorrowedBook(isbn)) {
            cout << "Book returned successfully.\n";
            user->saveAccountData(); // Persist updated record
        } else {
            cout << "Error in returning the book.\n";
        }
    }
    

 
};

void readfile(const string &fname)
{
    content.clear();
    vector<string> row;
    string line, word;
    ifstream file(fname);

    if (!file.is_open())
    {
        cerr << "Error: Could not open file " << fname << " for reading!\n";
        return;
    }

    while (getline(file, line))
    {
        row.clear();
        stringstream str(line);
        while (getline(str, word, ','))
            row.push_back(word);
        if (!row.empty())
        {
            content.push_back(row);
        }
    }

    file.close();
}

void writefile(const vector<vector<string>> &par, const string &fname)
{
    ofstream fout(fname);

    if (!fout.is_open())
    {
        cerr << "Error: Could not open file " << fname << " for writing!\n";
        return;
    }

    for (const auto &x : par)
    {
        for (size_t i = 0; i < x.size(); i++)
        {
            fout << x[i];
            if (i < x.size() - 1)
                fout << ",";
        }
        fout << "\n";
    }

    fout.close();
}

void writefileappend(const vector<string> &par, const string &fname)
{
    ofstream fout(fname, ios::app);

    if (!fout.is_open())
    {
        cerr << "Error: Could not open file " << fname << " for appending!\n";
        return;
    }

    for (size_t i = 0; i < par.size(); i++)
    {
        fout << par[i];
        if (i < par.size() - 1)
            fout << ",";
    }
    fout << "\n";

    fout.close();
}

int main()
{
    Library library;

    string userType;
    cout << "Enter user type (Student/Faculty/Librarian): ";
    cin >> userType;

    User *user = nullptr;  // Polymorphic base class pointer

    if (userType == "Student")
        user = new Student();
    else if (userType == "Faculty")
        user = new Faculty();
    else if (userType == "Librarian")
        user = new Librarian();
    else
    {
        cout << "Invalid user type. Exiting...\n";
        return 1;
    }
    
    user->login();
    user->display_menu();

    delete user;
    return 0;
}











    // void Library::returnBook(const string &userId, const string &isbn) {
    //     auto userIt = find_if(users.begin(), users.end(), [&](const Account &acc) {
    //         return acc.getUserId() == userId;
    //     });
    
    //     if (userIt == users.end()) {
    //         cout << "Error: User not found!" << endl;
    //         return;
    //     }
    
    //     Account &user = *userIt;
    
    //     if (!user.hasBorrowed(isbn)) {
    //         cout << "Error: This book is not borrowed by the user!" << endl;
    //         return;
    //     }
    
    //     // Find the book and mark as not issued
    //     auto bookIt = find_if(books.begin(), books.end(), [&](const Book &book) {
    //         return book.getISBN() == isbn;
    //     });
    
    //     if (bookIt == books.end()) {
    //         cout << "Error: Book not found!" << endl;
    //         return;
    //     }
    
    //     bookIt->setIsIssued(false); // Mark the book as available
    
    //     // Retrieve borrow date and current date
    //     time_t borrowDate = user.getBorrowDate(isbn);
    //     time_t returnDate = time(0);
    
    //     // Remove book from user's borrowed list
    //     user.returnBook(isbn);
    
    //     // Log the return to borrow_history.csv
    //     ofstream historyFile("borrow_history.csv", ios::app);
    //     if (historyFile.is_open()) {
    //         historyFile << isbn << "," << userId << ","
    //                     << to_string(borrowDate) << "," << to_string(returnDate) << "\n";
    //         historyFile.close();
    //     } else {
    //         cout << "Error: Unable to open borrow_history.csv for logging!" << endl;
    //     }
    
    //     saveBooksToCSV();
    //     saveUsersToCSV();
    
    //     cout << "Book returned successfully and logged!" << endl;
    // }   



    
    // void initializeDefaultUsers()
    // {
    //     users.push_back(new Student("S001", "Alice", 1));
    //     users.push_back(new Student("S002", "Bob", 1));
    //     users.push_back(new Student("S003", "Charlie", 1));
    //     users.push_back(new Student("S004", "David", 1));
    //     users.push_back(new Student("S005", "Eve", 1));
    
    //     users.push_back(new Faculty("F001", "Prof. Xavier", 2));
    //     users.push_back(new Faculty("F002", "Dr. Strange", 2));
    //     users.push_back(new Faculty("F003", "Prof. McGonagall", 2));
    
    //     users.push_back(new Librarian("L001", "Admin", 3));
    // }
