#include "Account.h"

#include <thread>

std::string accountStatusToString(AccountStatus status) {
    switch (status) {
        case AccountStatus::ACTIVE:
            return "ACTIVE";

        case AccountStatus::CLOSED:
            return "CLOSED";
    }

    return "UNKNOWN";
}

Account::Account(
    int id,
    const std::string& accountName,
    const std::string& accountPin,
    long long initialBalance
)
    : accountId(id),
      name(accountName),
      pin(accountPin),
      balance(initialBalance),
      status(AccountStatus::ACTIVE) {
}

ReaderWriterLock& Account::getLock() {
    return accountLock;
}

int Account::getAccountId() const {
    return accountId;
}

const std::string& Account::getName() const {
    return name;
}

long long Account::getBalance() const {
    return balance;
}

AccountStatus Account::getStatus() const {
    return status;
}

bool Account::isActive() const {
    return status == AccountStatus::ACTIVE;
}

bool Account::verifyPin(const std::string& inputPin) const {
    return pin == inputPin;
}

void Account::changePin(const std::string& newPin) {
    pin = newPin;
}

void Account::deposit(long long amount) {
    balance += amount;
}

bool Account::withdraw(long long amount) {
    if (balance < amount) {
        return false;
    }

    balance -= amount;
    return true;
}

void Account::close() {
    status = AccountStatus::CLOSED;
}

void Account::unsafeDepositForRaceTest(long long amount) {
    long long oldBalance = balance;

    std::this_thread::yield();

    balance = oldBalance + amount;
}