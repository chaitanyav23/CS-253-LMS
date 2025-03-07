#include <bits/stdc++.h>
using namespace std;

vector<vector<string>> content;

void readfile(const string &fname);
void writefile(const vector<vector<string>> &par, const string &fname);
void writefileappend(const vector<string> &par, const string &fname);

string toLower(string str)
{
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

bool caseInsensitiveCompare(const string &str1, const string &str2)
{
    return toLower(str1) == toLower(str2);
}

class Account
{
private:
    vector<pair<string, time_t>> borrowedBooks; 
    int fineAmount;
    int maxBorrowLimit;
    int borrowPeriod;
    bool isFaculty;

    int calculateDays(time_t now, time_t past) const
    {
        return (now - past) / 86400; 
    }

public:
    Account(bool faculty = false) : fineAmount(0), isFaculty(faculty)
    {
        maxBorrowLimit = faculty ? 5 : 3;
        borrowPeriod = faculty ? 30 : 15;
    }

    void pay_fine();

    vector<pair<string, time_t>> getBorrowedBooks() const {
        return borrowedBooks;
    }

    void setBorrowedBooks(const vector<pair<string, time_t>>& books) {
        borrowedBooks = books;
    }
    

    bool addBorrowedBook(const string &isbn) {
        if ((int)borrowedBooks.size() >= maxBorrowLimit)
            return false;
        
        borrowedBooks.emplace_back(isbn, time(0));
        return true;
    }

    bool removeBorrowedBook(const string &isbn)
    {
        auto it = find_if(borrowedBooks.begin(), borrowedBooks.end(), [&](const pair<string, time_t> &book)
                          { return book.first == isbn; });

        if (it != borrowedBooks.end())
        {
            borrowedBooks.erase(it);
            calculateFine(); 
            return true;
        }
        return false;
    }

    int calculateFine() const
    {
        if (isFaculty)
            return 0;
    
        int totalFine = 0;
    
        time_t currentTime = time(0);
    
        for (const auto &book : borrowedBooks)
        {
            int daysBorrowed = calculateDays(currentTime, book.second);
    
            if (daysBorrowed > borrowPeriod)
            {
                totalFine += (daysBorrowed - borrowPeriod) * 10; 
            }
        }
    
        return totalFine;
    }
        

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
    int currentFine = calculateFine(); 

    return (int)borrowedBooks.size() < maxBorrowLimit &&
           currentFine == 0 &&                  
           (!isFaculty || !hasOverdue60Days()); 
}



bool hasOverdue60Days() const
{
    if (!isFaculty) return false; 

    time_t currentTime = time(0);

    for (const auto &book : borrowedBooks)
    {
        int daysBorrowed = calculateDays(currentTime, book.second);

        if (daysBorrowed > 60)
            return true;
    }

    return false;
}

void showBorrowedBooks() const {
    if (borrowedBooks.empty()) {
        cout << "No books currently borrowed.\n";
        return;
    }

    cout << "Borrowed Books (ISBN | Days Borrowed | Status):\n";
    time_t now = time(0);
    for (const auto &book : borrowedBooks) {
        int daysBorrowed = calculateDays(now, book.second);
        cout << book.first << " | " << daysBorrowed << " days";

        if (daysBorrowed > borrowPeriod) cout << " | OVERDUE";
        cout << "\n";
    }
}

bool hasBorrowed(const string &isbn) const {
    return any_of(borrowedBooks.begin(), borrowedBooks.end(),
                  [&](const pair<string, time_t> &book) { return book.first == isbn; });
}

    int getFineAmount() const { return fineAmount; }
    int getBorrowedCount() const { return (int)borrowedBooks.size(); }
    int getMaxBorrowLimit() const { return maxBorrowLimit; }
};

class User
{
protected:
    string password;
    Account account; 

public:
    string name;
    string id;
    int role;

    User(string id, string name, string password, int role)
        : id(id), name(name), password(password), role(role) 
    {
        loadIssuedBooks(); 
    }
    string getId() const { return id; }
    string getName() const { return name; }
    int getRole() const { return role; }
    string getPassword() const { return password; }    

    string toCSV() const
    {
        return id + "," + name + "," + to_string(role);
    }

    virtual void login() = 0;
    virtual void display_menu() = 0;
    virtual void issue_book(const string &bookname) = 0;

    virtual ~User() {}

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

    void loadIssuedBooks() {
        readfile("issued_books_data.csv"); 
        vector<pair<string, time_t>> books; 

        for (const auto &entry : content) {
            for (const auto &field : entry) {
                cout << field << " | ";
            }
            cout << "\n";
        }
    
        for (const auto &entry : content) {
            if (entry.size() < 4) { 
                continue;
            }
    
            if (entry[0] == id) { 
                string isbn = entry[2]; 
                time_t issue_time;
    
                try {
                    issue_time = stol(entry[3]);
                } catch (const exception &e) {
                    continue;
                }
    
                books.emplace_back(isbn, issue_time);
            }
        }

        account.setBorrowedBooks(books);
    }
        
    
    void see_issued_books()
    {
        account.showBorrowedBooks();
    }
    
    void return_book(const string &isbn) {
        if (account.removeBorrowedBook(isbn)) {
            readfile("issued_books_data.csv");
    
            content.erase(remove_if(content.begin(), content.end(), [&](const vector<string> &entry) {
                return entry[0] == id && entry[2] == isbn; 
            }), content.end());
    
            writefile(content, "issued_books_data.csv");
            
            cout << "Book returned successfully.\n";
        } else {
            cout << "You have not issued this book.\n";
        }
    }
    

    void saveAccountData()
    {
        readfile("issued_books_data.csv");

        content.erase(remove_if(content.begin(), content.end(), [&](const vector<string> &entry)
                                { return entry[0] == id; }),
                      content.end());

        for (const auto &book : account.getBorrowedBooks())
        {
            content.push_back({id, name, book.first, to_string(book.second)});
        }

        writefile(content, "issued_books_data.csv");
    }


    void pay_fine() 
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

    void viewBorrowHistory() {
        readfile("borrow_history.csv");

        bool found = false;
        cout << "Borrow History (ISBN | Title | Author | Year | Return Date):\n";

        for (const auto &entry : content) {
            if (entry[0] == id) {  // Match User ID
                for (const auto &field : entry) {
                    cout << field << " | ";
                }
                cout << "\n";
                found = true;
            }
        }

        if (!found) {
            cout << "No borrowing history available for your account.\n";
        }
    }


    string getUserId() const { return id; }

    bool hasBorrowed(const string &isbn) const {
        return account.hasBorrowed(isbn);
    }

    Account &getAccount() { return account; }
    const Account &getAccount() const { return account; }

    static User *fromCSV(const string &line); 

};


class Student : public User
{
public:

    Student() : User("DefaultID", "DefaultName", "defaultPass", 1) {} 

    Student(const string &id, const string &name, const string &password, int role)
        : User(id, name, password, role) {}
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
                 << "6. Borrowed History\n"
                 << "7. Logout\n"
                 << "Enter choice: ";
            
            int choice;
            cin >> choice;
    
            if (cin.fail()) 
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
                cin >> ws;
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
                viewBorrowHistory();
                break;
            case 7:
                cout << "Logging out...\n";
                return;
            default:
                cout << "Invalid choice. Please try again.\n";
            }
        }
    }
    
    void issue_book(const string &bookname) {
        readfile("issued_books_data.csv");
        int book_count = 0;
    
        for (auto &entry : content) {
            if (entry[0] == id) { 
                book_count++;
                if (entry[1] == bookname) { 
                    cout << "You have already borrowed this book.\n";
                    return;
                }
            }
        }
    
        if (book_count >= 3) {
            cout << "You have already borrowed the maximum allowed books (3). Return a book to borrow another.\n";
            return;
        }
    
        if (account.calculateFine() > 0) {
            cout << "You have unpaid fines. Please clear them before borrowing.\n";
            return;
        }
    
        readfile("all_books_data.csv");
        bool book_found = false;
        string isbn;
    
        for (auto &book : content) {
            if (book[0] == bookname && book[4] == "0") {
                book[4] = "1"; 
                book_found = true;
                isbn = book[3];
                break;
            }
        }
    
        if (!book_found) {
            cout << "Book not available or does not exist.\n";
            return;
        }
    
        writefile(content, "all_books_data.csv");
    
        writefileappend({id, bookname, isbn, to_string(time(0))}, "issued_books_data.csv");
    
        account.addBorrowedBook(isbn);
    
        cout << "Book issued successfully: " << bookname << " (ISBN: " << isbn << ")\n";
    }

};

class Faculty : public User
{
public:

    Faculty() : User("DefaultID", "DefaultName", "defaultPass", 2) {} 

    Faculty(const string &id, const string &name, const string &password, int role)
        : User(id, name, password, role) {}
    void login() override
    {
        cout << "Faculty login\n";
    }
    void display_menu() override
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
                viewBorrowHistory();
            }
            else if (choice == 7)
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
    void issue_book(const string &bookname) override {
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
        string isbn;
    
        for (auto &book : content) {
            if (strcasecmp(book[0].c_str(), bookname.c_str()) == 0 && book[4] == "0") {
                book_found = true;
                book[4] = "1"; 
                isbn = book[3];
                break;
            }
        }
    
        if (!book_found) {
            cout << "Book not available or does not exist.\n";
            return;
        }
    
        writefile(content, "all_books_data.csv");
        writefileappend({id, bookname, isbn, to_string(current_time)}, "issued_books_data.csv");
    
        account.addBorrowedBook(isbn); 
    
        cout << "Book issued successfully: " << bookname << " (ISBN: " << isbn << ")\n";
    }
        
};

class Librarian : public User
{
public:

    Librarian() : User("DefaultID", "DefaultName", "defaultPass", 3) {} 
    
    Librarian(const string &id, const string &name, const string &password, int role)
        : User(id, name, password, role) {}

    void login() override
    {
        cout << "Librarian login\n";
    }
    void display_menu()
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
    void issue_book(const string &bookname) override { cout << "Librarians cannot issue books.\n"; }
    void add_book();
    void remove_book();
    void update_book();
    void add_user();
    void remove_user();
    void update_user();
};


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
    readfile("all_users_data.csv");
    for (const auto &user : content)
    {
        if (user[1] == id)
        {
            cout << "Error: This ID is already registered. Please enter a different ID.\n";
            return;
        }
    }
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

User *User::fromCSV(const string &line)
{
    stringstream ss(line);
    string name, id, password, roleStr;

    if (!getline(ss, name, ',') || !getline(ss, id, ',') || !getline(ss, password, ',') || !getline(ss, roleStr, ','))
    {
        cerr << "Error: Malformed CSV line (missing fields): " << line << endl;
        return nullptr; 
    }

    if (roleStr.empty() || !all_of(roleStr.begin(), roleStr.end(), ::isdigit))
    {
        cerr << "Error: Invalid role value in CSV: " << roleStr << " in line: " << line << endl;
        return nullptr;
    }

    int role = stoi(roleStr); 

    if (role == 1)
        return new Student(id, name, password, role);  
    else if (role == 2)
        return new Faculty(id, name, password, role);
    else if (role == 3)
        return new Librarian(id, name, password, role);
    else
    {
        cerr << "Error: Unknown role type in CSV: " << role << endl;
        return nullptr;
    }
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

    string getISBN() const { return isbn; }
    string getTitle() const { return title; }
    string getAuthor() const { return author; }
    int getYear() const { return year; }
    bool getIsIssued() const { return isIssued; }

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
            exit(1);
        }
    }

    void saveUsers()
    {
        ofstream file("all_users_data.csv");
        for (const auto &user : users) 
        {
            file << user->toCSV() << "\n"; 
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

    bool isIsbnDuplicate(const string &isbn) {
        for (const auto &book : books) {
            if (book.getISBN() == isbn) {
                return true; 
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

    void addBook() {
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
    
        if (isIsbnDuplicate(isbn)) {
            cout << "Error: A book with ISBN " << isbn << " already exists!" << endl;
            return;
        }
    
        Book newBook(isbn, title, author, stoi(year), true);
        books.push_back(newBook);

        saveBooks();
        cout << "Book added successfully!" << endl;
    }    

    void updateBook() {
        string isbn;
        cout << "Enter ISBN of the book to update: ";
        cin >> isbn;
    
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

    void returnBook(const string &userId, const string &isbn) {
        auto userIt = find_if(users.begin(), users.end(),
                              [&](const User *user) { return user->getUserId() == userId; });
    
        if (userIt == users.end()) {
            cout << "Error: User not found!" << endl;
            return;
        }
    
        User *user = *userIt;
    
        if (!user->hasBorrowed(isbn)) {
            cout << "Error: This book is not borrowed by the user!" << endl;
            return;
        }
    
        if (user->getAccount().removeBorrowedBook(isbn)) {
            cout << "Book returned successfully.\n";
            user->saveAccountData(); 
        } else {
            cout << "Error in returning the book.\n";
        }
    }
    
    void viewBorrowHistory() {
        readfile("borrow_history.csv");
    
        if (content.empty()) {
            cout << "No borrowing history available.\n";
            return;
        }
    
        cout << "Borrow History (ISBN | Title | Author | Year | Return Date):\n";
        for (const auto &entry : content) {
            for (const auto &field : entry) {
                cout << field << " | ";
            }
            cout << "\n";
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

    string userType, userId, password;
    cout << "Enter user type (Student/Faculty/Librarian): ";
    cin >> userType;
    
    cout << "Enter User ID: ";
    cin >> userId;

    cout << "Enter Password: ";
    cin >> password;

    ifstream file("all_users_data.csv");
    string line;
    User *user = nullptr;

    while (getline(file, line))
    {
        User *tempUser = User::fromCSV(line);
        if (tempUser && tempUser->getId() == userId && tempUser->getPassword() == password)
        {
            user = tempUser; 
            break;
        }
        delete tempUser;
    }
    file.close();

    if (!user)
    {
        cout << "Invalid credentials. Exiting...\n";
        return 1;
    }

    cout << userType << " login successful!\n";
    user->display_menu();

    delete user; 
    return 0;
}
