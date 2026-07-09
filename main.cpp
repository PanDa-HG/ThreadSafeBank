#include <iostream>
#include <unordered_map>
#include <string>
#include <sstream>
#include <cctype>
#include <vector>
#include <ctime>

using namespace std;

enum class TransactionType {
    DEPOSIT,
    WITHDRAW,
    TRANSFER
};

enum class TransactionStatus {
    SUCCESS,
    FAILED
};

enum class FailureReason {
    NONE,
    INVALID_ACCOUNT,
    INVALID_PIN,
    INSUFFICIENT_BALANCE,
    INVALID_AMOUNT,
    SAME_ACCOUNT
};

string transactionTypeToString(TransactionType type) {
    if (type == TransactionType::DEPOSIT) return "DEPOSIT";
    if (type == TransactionType::WITHDRAW) return "WITHDRAW";
    if (type == TransactionType::TRANSFER) return "TRANSFER";

    return "UNKNOWN";
}

string transactionStatusToString(TransactionStatus status) {
    if (status == TransactionStatus::SUCCESS) return "SUCCESS";
    if (status == TransactionStatus::FAILED) return "FAILED";

    return "UNKNOWN";
}

string failureReasonToString(FailureReason reason) {
    if (reason == FailureReason::NONE) return "NONE";
    if (reason == FailureReason::INVALID_ACCOUNT) return "INVALID_ACCOUNT";
    if (reason == FailureReason::INVALID_PIN) return "INVALID_PIN";
    if (reason == FailureReason::INSUFFICIENT_BALANCE) return "INSUFFICIENT_BALANCE";
    if (reason == FailureReason::INVALID_AMOUNT) return "INVALID_AMOUNT";
    if (reason == FailureReason::SAME_ACCOUNT) return "SAME_ACCOUNT";

    return "UNKNOWN";
}

string getCurrentTimestamp() {
    time_t now = time(nullptr);
    tm* localTime = localtime(&now);

    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localTime);

    return string(buffer);
}

struct Transaction {
    int transactionId;
    string timestamp;

    TransactionType type;
    int fromAccountId;
    int toAccountId;

    long long amount;

    TransactionStatus status;
    FailureReason reason;
};

class TransactionLog {
private:
    vector<Transaction> transactions;
    int nextTransactionId;

public:
    TransactionLog() {
        nextTransactionId = 1;
    }

    void addTransaction(TransactionType type,
                        int fromAccountId,
                        int toAccountId,
                        long long amount,
                        TransactionStatus status,
                        FailureReason reason) {
        Transaction txn;

        txn.transactionId = nextTransactionId;
        nextTransactionId++;

        txn.timestamp = getCurrentTimestamp();
        txn.type = type;
        txn.fromAccountId = fromAccountId;
        txn.toAccountId = toAccountId;
        txn.amount = amount;
        txn.status = status;
        txn.reason = reason;

        transactions.push_back(txn);
    }

    void printHistory(int accountId) {
        bool found = false;

        cout << "Transaction History for Account ID: " << accountId << "\n";

        for (const Transaction& txn : transactions) {
            if (txn.fromAccountId == accountId || txn.toAccountId == accountId) {
                found = true;

                cout << "----------------------------------------\n";
                cout << "Transaction ID: " << txn.transactionId << "\n";
                cout << "Timestamp: " << txn.timestamp << "\n";
                cout << "Type: " << transactionTypeToString(txn.type) << "\n";
                cout << "From Account: " << txn.fromAccountId << "\n";

                if (txn.toAccountId == -1) {
                    cout << "To Account: -\n";
                } else {
                    cout << "To Account: " << txn.toAccountId << "\n";
                }

                cout << "Amount: " << txn.amount << "\n";
                cout << "Status: " << transactionStatusToString(txn.status) << "\n";
                cout << "Reason: " << failureReasonToString(txn.reason) << "\n";
            }
        }

        if (!found) {
            cout << "No transactions found for this account.\n";
        }
    }
};

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
    TransactionLog transactionLog;
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

            transactionLog.addTransaction(
                TransactionType::DEPOSIT,
                accountId,
                -1,
                amount,
                TransactionStatus::FAILED,
                FailureReason::INVALID_AMOUNT
            );

            return false;
        }

        auto it = accounts.find(accountId);

        if (it == accounts.end()) {
            cout << "Deposit failed: Invalid account ID.\n";

            transactionLog.addTransaction(
                TransactionType::DEPOSIT,
                accountId,
                -1,
                amount,
                TransactionStatus::FAILED,
                FailureReason::INVALID_ACCOUNT
            );

            return false;
        }

        Account& account = it->second;
        account.deposit(amount);

        transactionLog.addTransaction(
            TransactionType::DEPOSIT,
            accountId,
            -1,
            amount,
            TransactionStatus::SUCCESS,
            FailureReason::NONE
        );

        cout << "Deposit successful.\n";
        cout << "Account ID: " << accountId << "\n";
        cout << "Deposited Amount: " << amount << "\n";
        cout << "Updated Balance: " << account.getBalance() << "\n";

        return true;
    }

    bool withdraw(int accountId, const string& pin, long long amount) {
        if (!isValidAmount(amount)) {
            cout << "Withdraw failed: Amount must be positive.\n";

            transactionLog.addTransaction(
                TransactionType::WITHDRAW,
                accountId,
                -1,
                amount,
                TransactionStatus::FAILED,
                FailureReason::INVALID_AMOUNT
            );

            return false;
        }

        auto it = accounts.find(accountId);

        if (it == accounts.end()) {
            cout << "Withdraw failed: Invalid account ID.\n";

            transactionLog.addTransaction(
                TransactionType::WITHDRAW,
                accountId,
                -1,
                amount,
                TransactionStatus::FAILED,
                FailureReason::INVALID_ACCOUNT
            );

            return false;
        }

        Account& account = it->second;

        if (!account.verifyPin(pin)) {
            cout << "Withdraw failed: Invalid PIN.\n";

            transactionLog.addTransaction(
                TransactionType::WITHDRAW,
                accountId,
                -1,
                amount,
                TransactionStatus::FAILED,
                FailureReason::INVALID_PIN
            );

            return false;
        }

        bool success = account.withdraw(amount);

        if (!success) {
            cout << "Withdraw failed: Insufficient balance.\n";

            transactionLog.addTransaction(
                TransactionType::WITHDRAW,
                accountId,
                -1,
                amount,
                TransactionStatus::FAILED,
                FailureReason::INSUFFICIENT_BALANCE
            );

            return false;
        }

        transactionLog.addTransaction(
            TransactionType::WITHDRAW,
            accountId,
            -1,
            amount,
            TransactionStatus::SUCCESS,
            FailureReason::NONE
        );

        cout << "Withdraw successful.\n";
        cout << "Account ID: " << accountId << "\n";
        cout << "Withdrawn Amount: " << amount << "\n";
        cout << "Updated Balance: " << account.getBalance() << "\n";

        return true;
    }

    bool transfer(int fromAccountId, const string& pin, int toAccountId, long long amount) {
        if (!isValidAmount(amount)) {
            cout << "Transfer failed: Amount must be positive.\n";

            transactionLog.addTransaction(
                TransactionType::TRANSFER,
                fromAccountId,
                toAccountId,
                amount,
                TransactionStatus::FAILED,
                FailureReason::INVALID_AMOUNT
            );

            return false;
        }

        auto fromIt = accounts.find(fromAccountId);

        if (fromIt == accounts.end()) {
            cout << "Transfer failed: Source account ID is invalid.\n";

            transactionLog.addTransaction(
                TransactionType::TRANSFER,
                fromAccountId,
                toAccountId,
                amount,
                TransactionStatus::FAILED,
                FailureReason::INVALID_ACCOUNT
            );

            return false;
        }

        auto toIt = accounts.find(toAccountId);

        if (toIt == accounts.end()) {
            cout << "Transfer failed: Destination account ID is invalid.\n";

            transactionLog.addTransaction(
                TransactionType::TRANSFER,
                fromAccountId,
                toAccountId,
                amount,
                TransactionStatus::FAILED,
                FailureReason::INVALID_ACCOUNT
            );

            return false;
        }

        if (fromAccountId == toAccountId) {
            cout << "Transfer failed: Source and destination account cannot be same.\n";

            transactionLog.addTransaction(
                TransactionType::TRANSFER,
                fromAccountId,
                toAccountId,
                amount,
                TransactionStatus::FAILED,
                FailureReason::SAME_ACCOUNT
            );

            return false;
        }

        Account& fromAccount = fromIt->second;
        Account& toAccount = toIt->second;

        if (!fromAccount.verifyPin(pin)) {
            cout << "Transfer failed: Invalid PIN.\n";

            transactionLog.addTransaction(
                TransactionType::TRANSFER,
                fromAccountId,
                toAccountId,
                amount,
                TransactionStatus::FAILED,
                FailureReason::INVALID_PIN
            );

            return false;
        }

        bool withdrawSuccess = fromAccount.withdraw(amount);

        if (!withdrawSuccess) {
            cout << "Transfer failed: Insufficient balance.\n";

            transactionLog.addTransaction(
                TransactionType::TRANSFER,
                fromAccountId,
                toAccountId,
                amount,
                TransactionStatus::FAILED,
                FailureReason::INSUFFICIENT_BALANCE
            );

            return false;
        }

        toAccount.deposit(amount);

        transactionLog.addTransaction(
            TransactionType::TRANSFER,
            fromAccountId,
            toAccountId,
            amount,
            TransactionStatus::SUCCESS,
            FailureReason::NONE
        );

        cout << "Transfer successful.\n";
        cout << "From Account: " << fromAccountId << "\n";
        cout << "To Account: " << toAccountId << "\n";
        cout << "Amount: " << amount << "\n";
        cout << "Updated Source Balance: " << fromAccount.getBalance() << "\n";
        cout << "Updated Destination Balance: " << toAccount.getBalance() << "\n";

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

    bool history(int accountId, const string& pin) {
        auto it = accounts.find(accountId);

        if (it == accounts.end()) {
            cout << "History failed: Invalid account ID.\n";
            return false;
        }

        Account& account = it->second;

        if (!account.verifyPin(pin)) {
            cout << "History failed: Invalid PIN.\n";
            return false;
        }

        transactionLog.printHistory(accountId);

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

    if (ss >> extra) {
        return true;
    }

    return false;
}

void printHelp() {
    cout << "\nAvailable commands:\n";
    cout << "CREATE_ACCOUNT <name> <pin> <initialBalance>\n";
    cout << "DEPOSIT <accountId> <amount>\n";
    cout << "WITHDRAW <accountId> <pin> <amount>\n";
    cout << "TRANSFER <fromAccountId> <pin> <toAccountId> <amount>\n";
    cout << "BALANCE <accountId> <pin>\n";
    cout << "HISTORY <accountId> <pin>\n";
    cout << "CHANGE_PIN <accountId> <oldPin> <newPin>\n";
    cout << "HELP\n";
    cout << "QUIT or EXIT\n\n";

    cout << "Examples:\n";
    cout << "CREATE_ACCOUNT Sachit 123456 5000\n";
    cout << "CREATE_ACCOUNT Aman 654321 3000\n";
    cout << "DEPOSIT 1001 1000\n";
    cout << "WITHDRAW 1001 123456 500\n";
    cout << "TRANSFER 1001 123456 1002 1000\n";
    cout << "BALANCE 1001 123456\n";
    cout << "HISTORY 1001 123456\n";
    cout << "CHANGE_PIN 1001 123456 111111\n";
    cout << "BALANCE 1001 111111\n\n";
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
        else if (command == "TRANSFER") {
            int fromAccountId;
            string pin;
            int toAccountId;
            long long amount;

            if (!(ss >> fromAccountId >> pin >> toAccountId >> amount) || hasExtraInput(ss)) {
                cout << "Invalid command format.\n";
                cout << "Usage: TRANSFER <fromAccountId> <pin> <toAccountId> <amount>\n";
                continue;
            }

            bank.transfer(fromAccountId, pin, toAccountId, amount);
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
        else if (command == "HISTORY") {
            int accountId;
            string pin;

            if (!(ss >> accountId >> pin) || hasExtraInput(ss)) {
                cout << "Invalid command format.\n";
                cout << "Usage: HISTORY <accountId> <pin>\n";
                continue;
            }

            bank.history(accountId, pin);
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