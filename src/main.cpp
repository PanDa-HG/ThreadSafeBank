#include "BankService.h"

#include <cctype>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

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

    cout << "TRANSFER <fromAccountId> <pin> "
         << "<toAccountId> <amount>\n";

    cout << "BALANCE <accountId> <pin>\n";
    cout << "HISTORY <accountId> <pin>\n";

    cout << "CHANGE_PIN <accountId> "
         << "<oldPin> <newPin>\n";

    cout << "CLOSE_ACCOUNT <accountId> <pin>\n";
    cout << "RUN_RACE_TEST\n";
    cout << "RUN_SAFE_TEST\n";
    cout << "RUN_TRANSFER_TEST\n";
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
    cout << "CLOSE_ACCOUNT 1001 111111\n";
    cout << "RUN_RACE_TEST\n";
    cout << "RUN_SAFE_TEST\n";
    cout << "RUN_TRANSFER_TEST\n\n";
}

int main() {
    BankService bank;

    cout << "Welcome to ThreadSafeBank!\n";
    cout << "Type HELP to see available commands.\n\n";

    string line;

    while (true) {
        cout << "bank> ";

        if (!getline(cin, line)) {
            cout << "\nInput closed. Exiting ThreadSafeBank.\n";
            break;
        }

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

            if (
                !(ss >> name >> pin >> initialBalance) ||
                hasExtraInput(ss)
            ) {
                cout << "Invalid command format.\n";
                cout << "Usage: CREATE_ACCOUNT "
                     << "<name> <pin> <initialBalance>\n";

                continue;
            }

            bank.createAccount(
                name,
                pin,
                initialBalance
            );
        }
        else if (command == "DEPOSIT") {
            int accountId;
            long long amount;

            if (
                !(ss >> accountId >> amount) ||
                hasExtraInput(ss)
            ) {
                cout << "Invalid command format.\n";
                cout << "Usage: DEPOSIT "
                     << "<accountId> <amount>\n";

                continue;
            }

            bank.deposit(accountId, amount);
        }
        else if (command == "WITHDRAW") {
            int accountId;
            string pin;
            long long amount;

            if (
                !(ss >> accountId >> pin >> amount) ||
                hasExtraInput(ss)
            ) {
                cout << "Invalid command format.\n";
                cout << "Usage: WITHDRAW "
                     << "<accountId> <pin> <amount>\n";

                continue;
            }

            bank.withdraw(
                accountId,
                pin,
                amount
            );
        }
        else if (command == "TRANSFER") {
            int fromAccountId;
            string pin;
            int toAccountId;
            long long amount;

            if (
                !(ss >> fromAccountId
                      >> pin
                      >> toAccountId
                      >> amount) ||
                hasExtraInput(ss)
            ) {
                cout << "Invalid command format.\n";

                cout << "Usage: TRANSFER "
                     << "<fromAccountId> <pin> "
                     << "<toAccountId> <amount>\n";

                continue;
            }

            bank.transfer(
                fromAccountId,
                pin,
                toAccountId,
                amount
            );
        }
        else if (command == "BALANCE") {
            int accountId;
            string pin;

            if (
                !(ss >> accountId >> pin) ||
                hasExtraInput(ss)
            ) {
                cout << "Invalid command format.\n";
                cout << "Usage: BALANCE "
                     << "<accountId> <pin>\n";

                continue;
            }

            bank.showBalance(accountId, pin);
        }
        else if (command == "HISTORY") {
            int accountId;
            string pin;

            if (
                !(ss >> accountId >> pin) ||
                hasExtraInput(ss)
            ) {
                cout << "Invalid command format.\n";
                cout << "Usage: HISTORY "
                     << "<accountId> <pin>\n";

                continue;
            }

            bank.history(accountId, pin);
        }
        else if (command == "CHANGE_PIN") {
            int accountId;
            string oldPin;
            string newPin;

            if (
                !(ss >> accountId >> oldPin >> newPin) ||
                hasExtraInput(ss)
            ) {
                cout << "Invalid command format.\n";

                cout << "Usage: CHANGE_PIN "
                     << "<accountId> <oldPin> <newPin>\n";

                continue;
            }

            bank.changePin(
                accountId,
                oldPin,
                newPin
            );
        }
        else if (command == "CLOSE_ACCOUNT") {
            int accountId;
            string pin;

            if (
                !(ss >> accountId >> pin) ||
                hasExtraInput(ss)
            ) {
                cout << "Invalid command format.\n";

                cout << "Usage: CLOSE_ACCOUNT "
                     << "<accountId> <pin>\n";

                continue;
            }

            bank.closeAccount(accountId, pin);
        }
        else if (command == "RUN_RACE_TEST") {
            if (hasExtraInput(ss)) {
                cout << "Invalid command format.\n";
                cout << "Usage: RUN_RACE_TEST\n";

                continue;
            }

            bank.runRaceTest();
        }
        else if (command == "RUN_SAFE_TEST") {
            if (hasExtraInput(ss)) {
                cout << "Invalid command format.\n";
                cout << "Usage: RUN_SAFE_TEST\n";

                continue;
            }

            bank.runSafeTest();
        }
        else if (command == "RUN_TRANSFER_TEST") {
            if (hasExtraInput(ss)) {
                cout << "Invalid command format.\n";
                cout << "Usage: RUN_TRANSFER_TEST\n";

                continue;
            }

            bank.runTransferTest();
        }
        else if (command == "HELP") {
            if (hasExtraInput(ss)) {
                cout << "Invalid command format.\n";
                cout << "Usage: HELP\n";

                continue;
            }

            printHelp();
        }
        else if (
            command == "QUIT" ||
            command == "EXIT"
        ) {
            if (hasExtraInput(ss)) {
                cout << "Invalid command format.\n";
                cout << "Usage: QUIT or EXIT\n";

                continue;
            }

            cout << "Exiting ThreadSafeBank. Goodbye!\n";
            break;
        }
        else {
            cout << "Unknown command: "
                 << command << "\n";

            cout << "Type HELP to see available commands.\n";
        }

        cout << "\n";
    }

    return 0;
}