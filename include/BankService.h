#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "Account.h"
#include "TransactionLog.h"

class BankService {
private:
    std::unordered_map<int, std::unique_ptr<Account>> accounts;
    TransactionLog transactionLog;
    int nextAccountId;
    std::mutex bankMutex;

    bool isValidPin(const std::string& pin) const;
    bool isValidAmount(long long amount) const;

    Account* getAccountById(int accountId);

public:
    BankService();

    BankService(const BankService&) = delete;
    BankService& operator=(const BankService&) = delete;

    int createAccount(
        const std::string& name,
        const std::string& pin,
        long long initialBalance
    );

    bool deposit(
        int accountId,
        long long amount
    );

    bool withdraw(
        int accountId,
        const std::string& pin,
        long long amount
    );

    bool transfer(
        int fromAccountId,
        const std::string& pin,
        int toAccountId,
        long long amount
    );

    bool showBalance(
        int accountId,
        const std::string& pin
    );

    bool changePin(
        int accountId,
        const std::string& oldPin,
        const std::string& newPin
    );

    bool closeAccount(
        int accountId,
        const std::string& pin
    );

    bool history(
        int accountId,
        const std::string& pin
    );

    void runRaceTest();
    void runSafeTest();
    void runTransferTest();
};