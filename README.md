# Library Management System (LMS)

## 📌 Overview
This Library Management System (LMS) is a C++ program that allows students, faculty, and librarians to manage books in a library. Users can:
- View available books
- Borrow and return books
- Track issued books
- Pay fines (if applicable)
- Maintain a history of borrowed books

## 🚀 Features
- Role-based access for **Students, Faculty, and Librarians**
- Book issuing and returning system
- Automatic fine calculation for overdue books
- Borrow history tracking
- Admin functions for adding and removing books/users

---

## 🛠 Installation & Running Instructions
### **1. Compile the Program**
Use the following command to compile the program:
```sh
 g++ -o b lms.cpp
```

### **2. Run the Program**
```sh
 ./b
```

### **3. Login as a User**
- **Student** - Limited to borrowing **3 books** for **15 days**
- **Faculty** - Limited to borrowing **5 books** for **30 days**
- **Librarian** - Can add/remove users and books

Enter the correct **User ID** and **Password** when prompted.

---

## 📂 File Structure
- `lms.cpp` - Main source code
- `all_books_data.csv` - Stores details of all books
- `all_users_data.csv` - Stores registered users
- `issued_books_data.csv` - Tracks currently issued books
- `borrow_history.csv` - Maintains the borrowing history

---

## 🏗️ Class Overview
### **1️⃣ Account**
Manages borrowed books and fines.
- `addBorrowedBook(isbn)` - Issues a book
- `removeBorrowedBook(isbn)` - Returns a book
- `calculateFine()` - Computes overdue fines
- `hasOverdue60Days()` - Restricts faculty if overdue exceeds 60 days
- `showBorrowedBooks()` - Displays issued books

### **2️⃣ User (Base Class)**
Parent class for `Student`, `Faculty`, and `Librarian`.
- `login()` - Handles user authentication
- `display_menu()` - Provides user options
- `see_all_books()` - Displays available books
- `see_issued_books()` - Shows issued books
- `return_book(isbn)` - Returns a book
- `pay_fine()` - Handles fine payments
- `viewBorrowHistory()` - Displays borrowing history

### **3️⃣ Student (Derived from User)**
- Can borrow **up to 3 books** for **15 days**
- Cannot issue books if there are unpaid fines

### **4️⃣ Faculty (Derived from User)**
- Can borrow **up to 5 books** for **30 days**
- Cannot issue books if overdue for **more than 60 days**

### **5️⃣ Librarian (Derived from User)**
- **Manages books and users**
- `add_book()` - Adds a new book
- `remove_book()` - Removes a book
- `add_user()` - Registers a new user
- `remove_user()` - Deletes a user
- `update_book()` - Edits book details
- `update_user()` - Modifies user details

### **6️⃣ Library**
Handles book management.
- `displayBooks()` - Lists all books
- `returnBook(userId, isbn)` - Processes book returns
- `logReturn(book)` - Updates `borrow_history.csv`

---

## 🛠️ How It Works
1. **Login as a User** - Enter **User ID** and **Password**
2. **Choose an Action** - Issue, return, pay fines, etc.
3. **Issuing a Book**
   - Checks book availability
   - Ensures borrowing limit is not exceeded
   - Adds entry to `issued_books_data.csv`
4. **Returning a Book**
   - Removes entry from `issued_books_data.csv`
   - Updates `borrow_history.csv`
5. **Viewing Borrow History**
   - Reads from `borrow_history.csv` to show past transactions

---

## 🛠 Troubleshooting
### ❓ Cannot Issue a Book?
- Check if you've exceeded the borrowing limit
- Ensure you have no unpaid fines
- Verify if the book is available

### ❓ Cannot Return a Book?
- Ensure the book is in your **issued books list**

### ❓ Fine Not Updating?
- Ensure system time is correct
- Try logging out and logging back in

---

## 📌 Future Improvements
- Implement **Graphical User Interface (GUI)**
- Add **Search & Filter** options for books
- Implement **Database (SQLite/MySQL)** instead of CSV files

---

## 📝 Conclusion
This Library Management System provides **role-based access**, efficient book tracking, and fine management. It ensures a smooth library experience for students, faculty, and librarians.

