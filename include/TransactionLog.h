#pragma once

#include <mutex>
#include <vector>

#include "Transaction.h"

class TransactionLog {
private:
    std::vector<Transaction> transactions;
    int nextTransactionId;
    std::mutex logMutex;

public:
    TransactionLog();

    TransactionLog(const TransactionLog&) = delete;
    TransactionLog& operator=(const TransactionLog&) = delete;

    void addTransaction(
        TransactionType type,
        int fromAccountId,
        int toAccountId,
        long long amount,
        TransactionStatus status,
        FailureReason reason
    );

    void printHistory(int accountId);
};