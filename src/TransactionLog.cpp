#include "TransactionLog.h"

#include <ctime>
#include <iostream>
#include <string>

namespace {
    std::string getCurrentTimestamp() {
        std::time_t now = std::time(nullptr);
        std::tm* localTime = std::localtime(&now);

        char buffer[80];

        std::strftime(
            buffer,
            sizeof(buffer),
            "%Y-%m-%d %H:%M:%S",
            localTime
        );

        return std::string(buffer);
    }
}

TransactionLog::TransactionLog()
    : nextTransactionId(1) {
}

void TransactionLog::addTransaction(
    TransactionType type,
    int fromAccountId,
    int toAccountId,
    long long amount,
    TransactionStatus status,
    FailureReason reason
) {
    std::unique_lock<std::mutex> lock(logMutex);

    Transaction transaction;

    transaction.transactionId = nextTransactionId;
    nextTransactionId++;

    transaction.timestamp = getCurrentTimestamp();
    transaction.type = type;
    transaction.fromAccountId = fromAccountId;
    transaction.toAccountId = toAccountId;
    transaction.amount = amount;
    transaction.status = status;
    transaction.reason = reason;

    transactions.push_back(transaction);
}

void TransactionLog::printHistory(int accountId) {
    std::unique_lock<std::mutex> lock(logMutex);

    bool found = false;

    std::cout
        << "\nTransaction History for Account ID: "
        << accountId
        << "\n";

    for (const Transaction& transaction : transactions) {
        if (
            transaction.fromAccountId == accountId ||
            transaction.toAccountId == accountId
        ) {
            found = true;

            std::cout << "----------------------------------------\n";

            std::cout
                << "Transaction ID: "
                << transaction.transactionId
                << "\n";

            std::cout
                << "Timestamp: "
                << transaction.timestamp
                << "\n";

            std::cout
                << "Type: "
                << transactionTypeToString(transaction.type)
                << "\n";

            std::cout
                << "From Account: "
                << transaction.fromAccountId
                << "\n";

            if (transaction.toAccountId == -1) {
                std::cout << "To Account: -\n";
            } else {
                std::cout
                    << "To Account: "
                    << transaction.toAccountId
                    << "\n";
            }

            std::cout
                << "Amount: "
                << transaction.amount
                << "\n";

            std::cout
                << "Status: "
                << transactionStatusToString(transaction.status)
                << "\n";

            std::cout
                << "Reason: "
                << failureReasonToString(transaction.reason)
                << "\n";
        }
    }

    if (!found) {
        std::cout << "No transactions found for this account.\n";
    }
}