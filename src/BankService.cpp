#include "BankService.h"

#include <cctype>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

using namespace std;

BankService::BankService()
    : nextAccountId(1001) {
}

bool BankService::isValidPin(const string& pin) const {
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

bool BankService::isValidAmount(long long amount) const {
    return amount > 0;
}

Account* BankService::getAccountById(int accountId) {
    unique_lock<mutex> lock(bankMutex);

    auto it = accounts.find(accountId);

    if (it == accounts.end()) {
        return nullptr;
    }

    return it->second.get();
}

int BankService::createAccount(
    const string& name,
    const string& pin,
    long long initialBalance
) {
    if (!isValidPin(pin)) {
        cout << "Account creation failed: "
             << "PIN must be exactly 6 digits.\n";

        return -1;
    }

    if (initialBalance < 0) {
        cout << "Account creation failed: "
             << "Initial balance cannot be negative.\n";

        return -1;
    }

    int accountId;

    {
        unique_lock<mutex> lock(bankMutex);

        accountId = nextAccountId;
        nextAccountId++;

        accounts[accountId] = make_unique<Account>(
            accountId,
            name,
            pin,
            initialBalance
        );
    }

    cout << "Account created successfully.\n";
    cout << "Account ID: " << accountId << "\n";
    cout << "Name: " << name << "\n";
    cout << "Initial Balance: " << initialBalance << "\n";
    cout << "Status: ACTIVE\n";

    return accountId;
}

bool BankService::deposit(
    int accountId,
    long long amount
) {
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

    Account* account = getAccountById(accountId);

    if (account == nullptr) {
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

    {
        WriteLockGuard guard(account->getLock());

        if (!account->isActive()) {
            cout << "Deposit failed: Account is closed.\n";

            transactionLog.addTransaction(
                TransactionType::DEPOSIT,
                accountId,
                -1,
                amount,
                TransactionStatus::FAILED,
                FailureReason::ACCOUNT_CLOSED
            );

            return false;
        }

        account->deposit(amount);

        cout << "Deposit successful.\n";
        cout << "Account ID: " << accountId << "\n";
        cout << "Deposited Amount: " << amount << "\n";
        cout << "Updated Balance: "
             << account->getBalance() << "\n";
    }

    transactionLog.addTransaction(
        TransactionType::DEPOSIT,
        accountId,
        -1,
        amount,
        TransactionStatus::SUCCESS,
        FailureReason::NONE
    );

    return true;
}

bool BankService::withdraw(
    int accountId,
    const string& pin,
    long long amount
) {
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

    Account* account = getAccountById(accountId);

    if (account == nullptr) {
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

    {
        WriteLockGuard guard(account->getLock());

        if (!account->verifyPin(pin)) {
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

        if (!account->isActive()) {
            cout << "Withdraw failed: Account is closed.\n";

            transactionLog.addTransaction(
                TransactionType::WITHDRAW,
                accountId,
                -1,
                amount,
                TransactionStatus::FAILED,
                FailureReason::ACCOUNT_CLOSED
            );

            return false;
        }

        if (!account->withdraw(amount)) {
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

        cout << "Withdraw successful.\n";
        cout << "Account ID: " << accountId << "\n";
        cout << "Withdrawn Amount: " << amount << "\n";
        cout << "Updated Balance: "
             << account->getBalance() << "\n";
    }

    transactionLog.addTransaction(
        TransactionType::WITHDRAW,
        accountId,
        -1,
        amount,
        TransactionStatus::SUCCESS,
        FailureReason::NONE
    );

    return true;
}

bool BankService::transfer(
    int fromAccountId,
    const string& pin,
    int toAccountId,
    long long amount
) {
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

    if (fromAccountId == toAccountId) {
        cout << "Transfer failed: "
             << "Source and destination accounts cannot be same.\n";

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

    Account* fromAccount = getAccountById(fromAccountId);

    if (fromAccount == nullptr) {
        cout << "Transfer failed: "
             << "Source account ID is invalid.\n";

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

    Account* toAccount = getAccountById(toAccountId);

    if (toAccount == nullptr) {
        cout << "Transfer failed: "
             << "Destination account ID is invalid.\n";

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

    Account* firstAccount;
    Account* secondAccount;

    if (fromAccountId < toAccountId) {
        firstAccount = fromAccount;
        secondAccount = toAccount;
    } else {
        firstAccount = toAccount;
        secondAccount = fromAccount;
    }

    {
        WriteLockGuard firstGuard(firstAccount->getLock());
        WriteLockGuard secondGuard(secondAccount->getLock());

        if (!fromAccount->verifyPin(pin)) {
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

        if (!fromAccount->isActive()) {
            cout << "Transfer failed: "
                 << "Source account is closed.\n";

            transactionLog.addTransaction(
                TransactionType::TRANSFER,
                fromAccountId,
                toAccountId,
                amount,
                TransactionStatus::FAILED,
                FailureReason::ACCOUNT_CLOSED
            );

            return false;
        }

        if (!toAccount->isActive()) {
            cout << "Transfer failed: "
                 << "Destination account is closed.\n";

            transactionLog.addTransaction(
                TransactionType::TRANSFER,
                fromAccountId,
                toAccountId,
                amount,
                TransactionStatus::FAILED,
                FailureReason::ACCOUNT_CLOSED
            );

            return false;
        }

        if (!fromAccount->withdraw(amount)) {
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

        toAccount->deposit(amount);

        cout << "Transfer successful.\n";
        cout << "From Account: " << fromAccountId << "\n";
        cout << "To Account: " << toAccountId << "\n";
        cout << "Amount: " << amount << "\n";

        cout << "Updated Source Balance: "
             << fromAccount->getBalance() << "\n";

        cout << "Updated Destination Balance: "
             << toAccount->getBalance() << "\n";
    }

    transactionLog.addTransaction(
        TransactionType::TRANSFER,
        fromAccountId,
        toAccountId,
        amount,
        TransactionStatus::SUCCESS,
        FailureReason::NONE
    );

    return true;
}

bool BankService::showBalance(
    int accountId,
    const string& pin
) {
    Account* account = getAccountById(accountId);

    if (account == nullptr) {
        cout << "Balance check failed: Invalid account ID.\n";
        return false;
    }

    ReadLockGuard guard(account->getLock());

    if (!account->verifyPin(pin)) {
        cout << "Balance check failed: Invalid PIN.\n";
        return false;
    }

    if (!account->isActive()) {
        cout << "Balance check failed: Account is closed.\n";
        return false;
    }

    cout << "Balance details:\n";
    cout << "Account ID: "
         << account->getAccountId() << "\n";

    cout << "Name: "
         << account->getName() << "\n";

    cout << "Balance: "
         << account->getBalance() << "\n";

    cout << "Status: "
         << accountStatusToString(account->getStatus())
         << "\n";

    return true;
}

bool BankService::changePin(
    int accountId,
    const string& oldPin,
    const string& newPin
) {
    Account* account = getAccountById(accountId);

    if (account == nullptr) {
        cout << "PIN change failed: Invalid account ID.\n";
        return false;
    }

    WriteLockGuard guard(account->getLock());

    if (!account->verifyPin(oldPin)) {
        cout << "PIN change failed: Old PIN is incorrect.\n";
        return false;
    }

    if (!account->isActive()) {
        cout << "PIN change failed: Account is closed.\n";
        return false;
    }

    if (!isValidPin(newPin)) {
        cout << "PIN change failed: "
             << "New PIN must be exactly 6 digits.\n";

        return false;
    }

    if (oldPin == newPin) {
        cout << "PIN change failed: "
             << "New PIN cannot be same as old PIN.\n";

        return false;
    }

    account->changePin(newPin);

    cout << "PIN changed successfully.\n";
    cout << "Account ID: " << accountId << "\n";

    return true;
}

bool BankService::closeAccount(
    int accountId,
    const string& pin
) {
    Account* account = getAccountById(accountId);

    if (account == nullptr) {
        cout << "Close account failed: Invalid account ID.\n";

        transactionLog.addTransaction(
            TransactionType::CLOSE_ACCOUNT,
            accountId,
            -1,
            0,
            TransactionStatus::FAILED,
            FailureReason::INVALID_ACCOUNT
        );

        return false;
    }

    {
        CloseLockGuard guard(account->getLock());

        if (!account->verifyPin(pin)) {
            cout << "Close account failed: Invalid PIN.\n";

            transactionLog.addTransaction(
                TransactionType::CLOSE_ACCOUNT,
                accountId,
                -1,
                0,
                TransactionStatus::FAILED,
                FailureReason::INVALID_PIN
            );

            return false;
        }

        if (!account->isActive()) {
            cout << "Close account failed: "
                 << "Account is already closed.\n";

            transactionLog.addTransaction(
                TransactionType::CLOSE_ACCOUNT,
                accountId,
                -1,
                0,
                TransactionStatus::FAILED,
                FailureReason::ACCOUNT_CLOSED
            );

            return false;
        }

        if (account->getBalance() != 0) {
            cout << "Close account failed: "
                 << "Account balance must be zero before closing.\n";

            cout << "Current Balance: "
                 << account->getBalance() << "\n";

            transactionLog.addTransaction(
                TransactionType::CLOSE_ACCOUNT,
                accountId,
                -1,
                0,
                TransactionStatus::FAILED,
                FailureReason::NON_ZERO_BALANCE
            );

            return false;
        }

        account->close();

        cout << "Account closed successfully.\n";
        cout << "Account ID: " << accountId << "\n";
        cout << "Status: CLOSED\n";
    }

    transactionLog.addTransaction(
        TransactionType::CLOSE_ACCOUNT,
        accountId,
        -1,
        0,
        TransactionStatus::SUCCESS,
        FailureReason::NONE
    );

    return true;
}

bool BankService::history(
    int accountId,
    const string& pin
) {
    Account* account = getAccountById(accountId);

    if (account == nullptr) {
        cout << "History failed: Invalid account ID.\n";
        return false;
    }

    {
        ReadLockGuard guard(account->getLock());

        if (!account->verifyPin(pin)) {
            cout << "History failed: Invalid PIN.\n";
            return false;
        }
    }

    transactionLog.printHistory(accountId);

    return true;
}

void BankService::runRaceTest() {
    cout << "Running race condition demo...\n\n";

    int accountId;
    Account* account;

    {
        unique_lock<mutex> lock(bankMutex);

        accountId = nextAccountId;
        nextAccountId++;

        accounts[accountId] = make_unique<Account>(
            accountId,
            "RaceTestAccount",
            "000000",
            0
        );

        account = accounts[accountId].get();
    }

    const int numberOfThreads = 4;
    const int depositsPerThread = 100000;
    const long long depositAmount = 1;

    const long long expectedBalance =
        1LL *
        numberOfThreads *
        depositsPerThread *
        depositAmount;

    cout << "Test Account ID: " << accountId << "\n";
    cout << "Threads: " << numberOfThreads << "\n";
    cout << "Deposits per thread: "
         << depositsPerThread << "\n";
    cout << "Deposit amount: "
         << depositAmount << "\n\n";

    auto startTime =
        chrono::high_resolution_clock::now();

    vector<thread> workers;

    for (int i = 0; i < numberOfThreads; i++) {
        workers.emplace_back(
            [account, depositsPerThread, depositAmount]() {
                for (int j = 0; j < depositsPerThread; j++) {
                    account->unsafeDepositForRaceTest(
                        depositAmount
                    );
                }
            }
        );
    }

    for (thread& worker : workers) {
        worker.join();
    }

    auto endTime =
        chrono::high_resolution_clock::now();

    auto durationMs =
        chrono::duration_cast<chrono::milliseconds>(
            endTime - startTime
        ).count();

    long long actualBalance = account->getBalance();

    cout << "Expected Balance: "
         << expectedBalance << "\n";

    cout << "Actual Balance: "
         << actualBalance << "\n";

    cout << "Time Taken: "
         << durationMs << " ms\n\n";

    if (actualBalance != expectedBalance) {
        cout << "Race condition detected! "
             << "Some updates were lost.\n";
    } else {
        cout << "The balances matched this time.\n";
        cout << "Race conditions are timing-dependent, "
             << "so run the test again.\n";
    }
}

void BankService::runSafeTest() {
    cout << "Running safe account-level lock test...\n\n";

    int accountId;
    Account* account;

    {
        unique_lock<mutex> lock(bankMutex);

        accountId = nextAccountId;
        nextAccountId++;

        accounts[accountId] = make_unique<Account>(
            accountId,
            "SafeTestAccount",
            "000000",
            0
        );

        account = accounts[accountId].get();
    }

    const int numberOfThreads = 4;
    const int depositsPerThread = 100000;
    const long long depositAmount = 1;

    const long long expectedBalance =
        1LL *
        numberOfThreads *
        depositsPerThread *
        depositAmount;

    cout << "Test Account ID: " << accountId << "\n";
    cout << "Threads: " << numberOfThreads << "\n";
    cout << "Deposits per thread: "
         << depositsPerThread << "\n";
    cout << "Deposit amount: "
         << depositAmount << "\n\n";

    auto startTime =
        chrono::high_resolution_clock::now();

    vector<thread> workers;

    for (int i = 0; i < numberOfThreads; i++) {
        workers.emplace_back(
            [account, depositsPerThread, depositAmount]() {
                for (int j = 0; j < depositsPerThread; j++) {
                    WriteLockGuard guard(account->getLock());

                    account->unsafeDepositForRaceTest(
                        depositAmount
                    );
                }
            }
        );
    }

    for (thread& worker : workers) {
        worker.join();
    }

    auto endTime =
        chrono::high_resolution_clock::now();

    auto durationMs =
        chrono::duration_cast<chrono::milliseconds>(
            endTime - startTime
        ).count();

    long long actualBalance = account->getBalance();

    cout << "Expected Balance: "
         << expectedBalance << "\n";

    cout << "Actual Balance: "
         << actualBalance << "\n";

    cout << "Time Taken: "
         << durationMs << " ms\n\n";

    if (actualBalance == expectedBalance) {
        cout << "Account-level synchronization successful.\n";
        cout << "No updates were lost.\n";
    } else {
        cout << "Balance mismatch even after synchronization.\n";
    }
}

void BankService::runTransferTest() {
    cout << "Running concurrent transfer test...\n\n";

    const int numberOfAccounts = 8;
    const int numberOfThreads = 8;
    const int transfersPerThread = 50000;

    const long long initialBalance = 100000;
    const long long transferAmount = 1;

    vector<Account*> testAccounts;

    {
        unique_lock<mutex> lock(bankMutex);

        for (int i = 0; i < numberOfAccounts; i++) {
            int accountId = nextAccountId;
            nextAccountId++;

            accounts[accountId] = make_unique<Account>(
                accountId,
                "TransferTest" + to_string(i + 1),
                "000000",
                initialBalance
            );

            testAccounts.push_back(
                accounts[accountId].get()
            );
        }
    }

    long long totalBalanceBefore = 0;

    for (Account* account : testAccounts) {
        ReadLockGuard guard(account->getLock());
        totalBalanceBefore += account->getBalance();
    }

    vector<long long> successfulTransfers(
        numberOfThreads,
        0
    );

    vector<long long> failedTransfers(
        numberOfThreads,
        0
    );

    const long long attemptedTransfers =
        1LL * numberOfThreads * transfersPerThread;

    cout << "Accounts: "
         << numberOfAccounts << "\n";

    cout << "Initial Balance per Account: "
         << initialBalance << "\n";

    cout << "Threads: "
         << numberOfThreads << "\n";

    cout << "Transfers per Thread: "
         << transfersPerThread << "\n";

    cout << "Attempted Transfers: "
         << attemptedTransfers << "\n\n";

    auto startTime =
        chrono::high_resolution_clock::now();

    vector<thread> workers;

    for (
        int threadId = 0;
        threadId < numberOfThreads;
        threadId++
    ) {
        workers.emplace_back(
            [&, threadId]() {
                for (
                    int operation = 0;
                    operation < transfersPerThread;
                    operation++
                ) {
                    int fromIndex =
                        (threadId + operation) %
                        numberOfAccounts;

                    int toIndex =
                        (fromIndex + 1) %
                        numberOfAccounts;

                    Account* fromAccount =
                        testAccounts[fromIndex];

                    Account* toAccount =
                        testAccounts[toIndex];

                    Account* firstAccount;
                    Account* secondAccount;

                    if (
                        fromAccount->getAccountId() <
                        toAccount->getAccountId()
                    ) {
                        firstAccount = fromAccount;
                        secondAccount = toAccount;
                    } else {
                        firstAccount = toAccount;
                        secondAccount = fromAccount;
                    }

                    WriteLockGuard firstGuard(
                        firstAccount->getLock()
                    );

                    WriteLockGuard secondGuard(
                        secondAccount->getLock()
                    );

                    if (
                        fromAccount->withdraw(
                            transferAmount
                        )
                    ) {
                        toAccount->deposit(
                            transferAmount
                        );

                        successfulTransfers[threadId]++;
                    } else {
                        failedTransfers[threadId]++;
                    }
                }
            }
        );
    }

    for (thread& worker : workers) {
        worker.join();
    }

    auto endTime =
        chrono::high_resolution_clock::now();

    double durationSeconds =
        chrono::duration<double>(
            endTime - startTime
        ).count();

    long long totalSuccessfulTransfers = 0;
    long long totalFailedTransfers = 0;

    for (int i = 0; i < numberOfThreads; i++) {
        totalSuccessfulTransfers +=
            successfulTransfers[i];

        totalFailedTransfers +=
            failedTransfers[i];
    }

    long long totalBalanceAfter = 0;

    for (Account* account : testAccounts) {
        ReadLockGuard guard(account->getLock());
        totalBalanceAfter += account->getBalance();
    }

    double throughput = 0.0;

    if (durationSeconds > 0.0) {
        throughput =
            attemptedTransfers / durationSeconds;
    }

    cout << "Successful Transfers: "
         << totalSuccessfulTransfers << "\n";

    cout << "Failed Transfers: "
         << totalFailedTransfers << "\n\n";

    cout << "Total Balance Before: "
         << totalBalanceBefore << "\n";

    cout << "Total Balance After: "
         << totalBalanceAfter << "\n\n";

    cout << fixed << setprecision(3);

    cout << "Time Taken: "
         << durationSeconds << " seconds\n";

    cout << setprecision(0);

    cout << "Throughput: "
         << throughput
         << " transfers/second\n\n";

    if (totalBalanceBefore == totalBalanceAfter) {
        cout << "Concurrent transfer test passed.\n";
        cout << "No money was lost or created.\n";
        cout << "All worker threads completed successfully.\n";
    } else {
        cout << "Concurrent transfer test failed.\n";
        cout << "The total balance changed during transfers.\n";
    }

    cout << defaultfloat;
}