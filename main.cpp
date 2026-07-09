#include <iostream>
#include <unordered_map>
#include <string>
#include <sstream>
#include <cctype>

using namespace std;

class Account {
private:
    int accountId;
    string name;
    string pin;
    long long balance;

public:
    Account(int id, const string& accountName, const string& accountPin, long long initialBalance) {
        accountId = id;
        name = accountName;
        pin = accountPin;
        balance = initialBalance;
    }

    int getAccountId() const {
        return accountId;
    }

    string getName() const {
        return name;
    }

    long long getBalance() const {
        return balance;
    }

    bool verifyPin(const string& inputPin) const {
        return pin == inputPin;
    }

    void changePin(const string& newPin) {
        pin = newPin;
    }

    void deposit(long long amount) {
        balance += amount;
    }

    bool withdraw(long long amount) {
        if (balance < amount) {
            return false;
        }

        balance -= amount;
        return true;
    }
};

class BankService {
private:
    unordered_map<int, Account> accounts;
    int nextAccountId;

    bool isValidPin(const string& pin) {
        if (pin.size() != 6) {
            return false;
        }

        for (char ch : pin) {
            if (!isdigit(static_cast<unsigned char>(ch))) {
                return false;
            }
        }

        return true;
    }

    bool isValidAmount(long long amount) {
        return amount > 0;
    }

public:
    BankService() {
        nextAccountId = 1001;
    }

    int createAccount(const string& name, const string& pin, long long initialBalance) {
        if (!isValidPin(pin)) {
            cout << "Account creation failed: PIN must be exactly 6 digits.\n";
            return -1;
        }

        if (initialBalance < 0) {
            cout << "Account creation failed: Initial balance cannot be negative.\n";
            return -1;
        }

        int accountId = nextAccountId;
        nextAccountId++;

        Account newAccount(accountId, name, pin, initialBalance);
        accounts.emplace(accountId, newAccount);

        cout << "Account created successfully.\n";
        cout << "Account ID: " << accountId << "\n";
        cout << "Name: " << name << "\n";
        cout << "Initial Balance: " << initialBalance << "\n";

        return accountId;
    }

    bool deposit(int accountId, long long amount) {
        if (!isValidAmount(amount)) {
            cout << "Deposit failed: Amount must be positive.\n";
            return false;
        }

        auto it = accounts.find(accountId);

        if (it == accounts.end()) {
            cout << "Deposit failed: Invalid account ID.\n";
            return false;
        }

        Account& account = it->second;
        account.deposit(amount);

        cout << "Deposit successful.\n";
        cout << "Account ID: " << accountId << "\n";
        cout << "Deposited Amount: " << amount << "\n";
        cout << "Updated Balance: " << account.getBalance() << "\n";

        return true;
    }

    bool withdraw(int accountId, const string& pin, long long amount) {
        if (!isValidAmount(amount)) {
            cout << "Withdraw failed: Amount must be positive.\n";
            return false;
        }

        auto it = accounts.find(accountId);

        if (it == accounts.end()) {
            cout << "Withdraw failed: Invalid account ID.\n";
            return false;
        }

        Account& account = it->second;

        if (!account.verifyPin(pin)) {
            cout << "Withdraw failed: Invalid PIN.\n";
            return false;
        }

        bool success = account.withdraw(amount);

        if (!success) {
            cout << "Withdraw failed: Insufficient balance.\n";
            return false;
        }

        cout << "Withdraw successful.\n";
        cout << "Account ID: " << accountId << "\n";
        cout << "Withdrawn Amount: " << amount << "\n";
        cout << "Updated Balance: " << account.getBalance() << "\n";

        return true;
    }

    bool showBalance(int accountId, const string& pin) {
        auto it = accounts.find(accountId);

        if (it == accounts.end()) {
            cout << "Balance check failed: Invalid account ID.\n";
            return false;
        }

        Account& account = it->second;

        if (!account.verifyPin(pin)) {
            cout << "Balance check failed: Invalid PIN.\n";
            return false;
        }

        cout << "Balance details:\n";
        cout << "Account ID: " << account.getAccountId() << "\n";
        cout << "Name: " << account.getName() << "\n";
        cout << "Balance: " << account.getBalance() << "\n";

        return true;
    }

    bool changePin(int accountId, const string& oldPin, const string& newPin) {
        auto it = accounts.find(accountId);

        if (it == accounts.end()) {
            cout << "PIN change failed: Invalid account ID.\n";
            return false;
        }

        Account& account = it->second;

        if (!account.verifyPin(oldPin)) {
            cout << "PIN change failed: Old PIN is incorrect.\n";
            return false;
        }

        if (!isValidPin(newPin)) {
            cout << "PIN change failed: New PIN must be exactly 6 digits.\n";
            return false;
        }

        if (oldPin == newPin) {
            cout << "PIN change failed: New PIN cannot be same as old PIN.\n";
            return false;
        }

        account.changePin(newPin);

        cout << "PIN changed successfully.\n";
        cout << "Account ID: " << accountId << "\n";

        return true;
    }
};

string toUpperCase(string text) {
    for (char& ch : text) {
        ch = toupper(static_cast<unsigned char>(ch));
    }

    return text;
}

bool hasExtraInput(stringstream& ss) {
    string extra;
    if(ss >> extra) return true;
    return false;
}

void printHelp() {
    cout << "\nAvailable commands:\n";
    cout << "CREATE_ACCOUNT <name> <pin> <initialBalance>\n";
    cout << "DEPOSIT <accountId> <amount>\n";
    cout << "WITHDRAW <accountId> <pin> <amount>\n";
    cout << "BALANCE <accountId> <pin>\n";
    cout << "CHANGE_PIN <accountId> <oldPin> <newPin>\n";
    cout << "HELP\n";
    cout << "QUIT or EXIT\n\n";

    cout << "Examples:\n";
    cout << "CREATE_ACCOUNT Sachit 123456 5000\n";
    cout << "DEPOSIT 1001 1000\n";
    cout << "WITHDRAW 1001 123456 500\n";
    cout << "BALANCE 1001 123456\n";
    cout << "CHANGE_PIN 1001 123456 654321\n";
    cout << "BALANCE 1001 654321\n\n";
}

int main() {
    BankService bank;

    cout << "Welcome to ThreadSafeBank!\n";
    cout << "Type HELP to see available commands.\n\n";

    string line;

    while (true) {
        cout << "bank> ";
        getline(cin, line);

        if (line.empty()) {
            continue;
        }

        stringstream ss(line);

        string command;
        ss >> command;
        command = toUpperCase(command);

        if (command == "CREATE_ACCOUNT") {
            string name;
            string pin;
            long long initialBalance;

            if (!(ss >> name >> pin >> initialBalance) || hasExtraInput(ss)) {
                cout << "Invalid command format.\n";
                cout << "Usage: CREATE_ACCOUNT <name> <pin> <initialBalance>\n";
                continue;
            }

            bank.createAccount(name, pin, initialBalance);
        }
        else if (command == "DEPOSIT") {
            int accountId;
            long long amount;

            if (!(ss >> accountId >> amount) || hasExtraInput(ss)) {
                cout << "Invalid command format.\n";
                cout << "Usage: DEPOSIT <accountId> <amount>\n";
                continue;
            }

            bank.deposit(accountId, amount);
        }
        else if (command == "WITHDRAW") {
            int accountId;
            string pin;
            long long amount;

            if (!(ss >> accountId >> pin >> amount) || hasExtraInput(ss)) {
                cout << "Invalid command format.\n";
                cout << "Usage: WITHDRAW <accountId> <pin> <amount>\n";
                continue;
            }

            bank.withdraw(accountId, pin, amount);
        }
        else if (command == "BALANCE") {
            int accountId;
            string pin;

            if (!(ss >> accountId >> pin) || hasExtraInput(ss)) {
                cout << "Invalid command format.\n";
                cout << "Usage: BALANCE <accountId> <pin>\n";
                continue;
            }

            bank.showBalance(accountId, pin);
        }
        else if (command == "CHANGE_PIN") {
            int accountId;
            string oldPin;
            string newPin;

            if (!(ss >> accountId >> oldPin >> newPin) || hasExtraInput(ss)) {
                cout << "Invalid command format.\n";
                cout << "Usage: CHANGE_PIN <accountId> <oldPin> <newPin>\n";
                continue;
            }

            bank.changePin(accountId, oldPin, newPin);
        }
        else if (command == "HELP") {
            if (hasExtraInput(ss)) {
                cout << "Invalid command format.\n";
                cout << "Usage: HELP\n";
                continue;
            }

            printHelp();
        }
        else if (command == "QUIT" || command == "EXIT") {
            if (hasExtraInput(ss)) {
                cout << "Invalid command format.\n";
                cout << "Usage: QUIT or EXIT\n";
                continue;
            }

            cout << "Exiting ThreadSafeBank. Goodbye!\n";
            break;
        }
        else {
            cout << "Unknown command: " << command << "\n";
            cout << "Type HELP to see available commands.\n";
        }

        cout << "\n";
    }

    return 0;
}