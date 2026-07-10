#pragma once

#include <string>

#include "ReaderWriterLock.h"

enum class AccountStatus {
    ACTIVE,
    CLOSED
};

std::string accountStatusToString(AccountStatus status);

class Account {
private:
    int accountId;
    std::string name;
    std::string pin;
    long long balance;
    AccountStatus status;
    ReaderWriterLock accountLock;

public:
    Account(
        int id,
        const std::string& accountName,
        const std::string& accountPin,
        long long initialBalance
    );

    Account(const Account&) = delete;
    Account& operator=(const Account&) = delete;

    ReaderWriterLock& getLock();

    int getAccountId() const;
    const std::string& getName() const;
    long long getBalance() const;
    AccountStatus getStatus() const;

    bool isActive() const;
    bool verifyPin(const std::string& inputPin) const;

    void changePin(const std::string& newPin);
    void deposit(long long amount);
    bool withdraw(long long amount);
    void close();

    void unsafeDepositForRaceTest(long long amount);
};