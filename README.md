# ThreadSafeBank

ThreadSafeBank is a **concurrent, in-memory banking system written in C++17**. It was built to explore practical multithreading concepts such as race conditions, fine-grained locking, deadlock prevention, condition variables, RAII-based lock management, and safe object ownership.

The project provides a command-line interface for creating accounts, depositing and withdrawing money, transferring funds, changing PINs, viewing transaction history, and closing accounts.

> This is an educational systems project, not a production banking application.

---

## Features

- Create accounts with a unique account ID and 6-digit PIN
- Deposit and withdraw money
- Transfer funds between accounts
- Check account balance
- Change account PIN
- View transaction history
- Soft-close accounts while preserving their history
- Thread-safe transaction logging
- Fine-grained per-account synchronization
- Reader, writer, and account-closure lock modes
- RAII lock guards for automatic lock release
- Deadlock-safe transfers using deterministic lock ordering
- Race-condition, synchronized-deposit, and concurrent-transfer demonstrations
- Modular header/source file structure
- Account ownership managed with `std::unique_ptr`

---

## Concurrency Design

### Fine-grained account locking

Each account owns its own custom `ReaderWriterLock`.

This allows operations on unrelated accounts to proceed concurrently instead of protecting the entire bank with one global mutex.

Examples:

- Two balance checks on the same account may run together.
- A deposit or withdrawal requires exclusive access to that account.
- A transfer requires exclusive access to both involved accounts.
- Closing an account receives higher priority than ordinary readers and writers.

### RAII lock guards

The project uses custom guards:

- `ReadLockGuard`
- `WriteLockGuard`
- `CloseLockGuard`

A guard acquires the required lock when it is created and automatically releases it when it goes out of scope. This reduces the risk of forgetting to unlock during early returns or exceptions.

### Deadlock prevention during transfers

A transfer needs locks on two accounts. If different threads lock those accounts in opposite orders, a circular wait could occur.

ThreadSafeBank prevents this by always acquiring account locks in ascending account-ID order:

```text
smaller account ID -> larger account ID
```

All transfer operations follow the same ordering, removing the circular-wait condition.

### Bank-level mutex

A separate bank mutex protects the account container and account-ID generation.

The mutex is used only while:

- looking up an account,
- adding an account, or
- generating a new account ID.

Account operations themselves use account-level locks.

### Transaction-log mutex

The transaction log has its own mutex so multiple banking operations can safely append transaction records concurrently.

---

## Project Structure

```text
ThreadSafeBank/
├── include/
│   ├── Account.h
│   ├── BankService.h
│   ├── ReaderWriterLock.h
│   ├── Transaction.h
│   └── TransactionLog.h
│
├── src/
│   ├── Account.cpp
│   ├── BankService.cpp
│   ├── main.cpp
│   ├── ReaderWriterLock.cpp
│   ├── Transaction.cpp
│   └── TransactionLog.cpp
│
├── .gitignore
└── README.md
```

### Main components

| Component | Responsibility |
|---|---|
| `Account` | Stores account state and basic balance/PIN operations |
| `ReaderWriterLock` | Provides reader, writer, and closure synchronization |
| `Transaction` | Defines transaction types, status, and failure reasons |
| `TransactionLog` | Stores and prints thread-safe transaction history |
| `BankService` | Implements banking rules and concurrent operations |
| `main.cpp` | Parses CLI commands and calls `BankService` |

---

## Requirements

- C++17-compatible compiler
- POSIX thread support through `-pthread`
- Tested with `g++`

---

## Build

Run the following command from the project root:

```bash
g++ -std=c++17 -pthread -Iinclude \
src/main.cpp \
src/ReaderWriterLock.cpp \
src/Account.cpp \
src/Transaction.cpp \
src/TransactionLog.cpp \
src/BankService.cpp \
-o bank
```

On Windows with MinGW/MSYS2:

```powershell
g++ -std=c++17 -pthread -Iinclude src\main.cpp src\ReaderWriterLock.cpp src\Account.cpp src\Transaction.cpp src\TransactionLog.cpp src\BankService.cpp -o bank.exe
```

---

## Run

Linux/macOS:

```bash
./bank
```

Windows PowerShell:

```powershell
.\bank.exe
```

---

## Available Commands

| Command | Description |
|---|---|
| `CREATE_ACCOUNT <name> <pin> <initialBalance>` | Creates a new account |
| `DEPOSIT <accountId> <amount>` | Deposits money |
| `WITHDRAW <accountId> <pin> <amount>` | Withdraws money |
| `TRANSFER <fromId> <pin> <toId> <amount>` | Transfers money |
| `BALANCE <accountId> <pin>` | Displays account details |
| `HISTORY <accountId> <pin>` | Displays transaction history |
| `CHANGE_PIN <accountId> <oldPin> <newPin>` | Changes the account PIN |
| `CLOSE_ACCOUNT <accountId> <pin>` | Soft-closes an account |
| `RUN_RACE_TEST` | Demonstrates lost updates without synchronization |
| `RUN_SAFE_TEST` | Demonstrates synchronized concurrent deposits |
| `RUN_TRANSFER_TEST` | Runs concurrent transfers and checks balance conservation |
| `HELP` | Displays all available commands |
| `EXIT` or `QUIT` | Closes the application |

---

## Example Session

```text
CREATE_ACCOUNT Sachit 123456 5000
CREATE_ACCOUNT Aman 654321 3000
DEPOSIT 1001 1000
WITHDRAW 1001 123456 500
TRANSFER 1001 123456 1002 1000
BALANCE 1001 123456
HISTORY 1001 123456
CHANGE_PIN 1001 123456 111111
EXIT
```

---

## Account-Closure Rules

Account closure is implemented as a soft delete.

An account can be closed only when:

- the account ID exists,
- the PIN is correct,
- the account is currently active, and
- the balance is zero.

After closure:

- deposits, withdrawals, transfers, balance checks, and PIN changes are rejected;
- transaction history remains available for audit purposes;
- the account object remains stored safely in memory.

Keeping closed accounts in memory also ensures that previously returned non-owning account pointers do not become dangling pointers during program execution.

---

## Transaction Logging

Each deposit, withdrawal, transfer, and account-closure attempt records:

- transaction ID,
- timestamp,
- transaction type,
- source account,
- destination account when applicable,
- amount,
- success or failure status, and
- failure reason.

Possible failure reasons include:

- invalid account,
- invalid PIN,
- invalid amount,
- insufficient balance,
- same source and destination account,
- closed account, and
- non-zero balance during closure.

---

## Concurrency Demonstrations

### `RUN_RACE_TEST`

Runs concurrent unsynchronized deposits using an intentionally unsafe read-modify-write operation.

The expected and actual balances may differ because updates can be lost. The result is timing-dependent, so an incorrect implementation may occasionally appear correct.

### `RUN_SAFE_TEST`

Runs the same deposit workload using account-level write locking.

The expected and actual balances should match because each update is protected.

### `RUN_TRANSFER_TEST`

Runs concurrent transfers across multiple accounts and checks that:

- all worker threads complete,
- transfers use ordered account locking, and
- the total balance before and after the test is unchanged.

The current configuration performs:

```text
8 threads x 50,000 transfers = 400,000 attempted transfers
```

These commands are stress demonstrations for manual inspection; they are not a formal automated test suite.

---

## Important Design Decisions

### Why `std::unique_ptr<Account>`?

The bank exclusively owns every account, so `std::unique_ptr` expresses that ownership clearly and releases account memory automatically when the bank is destroyed.

### Why does account lookup return `Account*`?

`getAccountById()` returns a non-owning pointer. Ownership remains with the `std::unique_ptr` stored inside the bank.

The program does not erase account objects during execution, so the pointer remains valid after the bank-container mutex is released.

### Why keep a separate lock per account?

A single global mutex would be simpler and correct, but it would unnecessarily serialize unrelated operations. Per-account locking allows independent accounts to be processed concurrently.

### Why use condition variables?

The custom reader-writer lock needs threads to sleep while their required lock mode is unavailable and wake when the lock state changes. Condition variables avoid wasteful busy waiting.

---

## Limitations

- All accounts and transactions are stored only in memory.
- Data is lost when the program exits.
- PINs are stored as plain strings and are not cryptographically protected.
- The project has no networking, database, or multi-process support.
- The concurrency demonstrations are not a formal automated test suite.
- The custom reader-writer lock is educational and has not been formally verified.
- The project should not be used for real financial operations.

---

## Possible Future Improvements

- Automated unit and concurrency tests using GoogleTest or Catch2
- ThreadSanitizer validation on Linux
- Persistent account and transaction storage
- PIN hashing and stronger authentication
- CMake-based build configuration
- Structured operation-result objects instead of printing inside `BankService`
- Continuous integration with GitHub Actions
- Client-server architecture with a database-backed service

---

## What I Learned

This project was built to understand:

- race conditions and lost updates,
- mutexes and condition variables,
- reader-writer synchronization,
- lock granularity and contention,
- deadlock formation and prevention,
- RAII-based resource management,
- smart-pointer ownership,
- thread-safe logging,
- modular C++ project organization, and
- compilation and linking across multiple translation units.

---

## Disclaimer

ThreadSafeBank is an educational project created to study C++ concurrency and systems-design fundamentals. It is not intended to represent a secure or production-ready banking platform.