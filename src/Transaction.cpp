#include "Transaction.h"

std::string transactionTypeToString(TransactionType type) {
    switch (type) {
        case TransactionType::DEPOSIT:
            return "DEPOSIT";

        case TransactionType::WITHDRAW:
            return "WITHDRAW";

        case TransactionType::TRANSFER:
            return "TRANSFER";

        case TransactionType::CLOSE_ACCOUNT:
            return "CLOSE_ACCOUNT";
    }

    return "UNKNOWN";
}

std::string transactionStatusToString(TransactionStatus status) {
    switch (status) {
        case TransactionStatus::SUCCESS:
            return "SUCCESS";

        case TransactionStatus::FAILED:
            return "FAILED";
    }

    return "UNKNOWN";
}

std::string failureReasonToString(FailureReason reason) {
    switch (reason) {
        case FailureReason::NONE:
            return "NONE";

        case FailureReason::INVALID_ACCOUNT:
            return "INVALID_ACCOUNT";

        case FailureReason::INVALID_PIN:
            return "INVALID_PIN";

        case FailureReason::INSUFFICIENT_BALANCE:
            return "INSUFFICIENT_BALANCE";

        case FailureReason::INVALID_AMOUNT:
            return "INVALID_AMOUNT";

        case FailureReason::SAME_ACCOUNT:
            return "SAME_ACCOUNT";

        case FailureReason::ACCOUNT_CLOSED:
            return "ACCOUNT_CLOSED";

        case FailureReason::NON_ZERO_BALANCE:
            return "NON_ZERO_BALANCE";
    }

    return "UNKNOWN";
}