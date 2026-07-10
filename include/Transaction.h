#pragma once

#include <string>

enum class TransactionType {
    DEPOSIT,
    WITHDRAW,
    TRANSFER,
    CLOSE_ACCOUNT
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
    SAME_ACCOUNT,
    ACCOUNT_CLOSED,
    NON_ZERO_BALANCE
};

struct Transaction {
    int transactionId;
    std::string timestamp;
    TransactionType type;
    int fromAccountId;
    int toAccountId;
    long long amount;
    TransactionStatus status;
    FailureReason reason;
};

std::string transactionTypeToString(TransactionType type);

std::string transactionStatusToString(TransactionStatus status);

std::string failureReasonToString(FailureReason reason);