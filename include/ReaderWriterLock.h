#pragma once

#include <condition_variable>
#include <mutex>

class ReaderWriterLock {
private:
    int activeReaders;
    int waitingWriters;
    int waitingClosers;
    bool activeWriter;

    std::mutex mtx;
    std::condition_variable cv;

public:
    ReaderWriterLock();

    ReaderWriterLock(const ReaderWriterLock&) = delete;
    ReaderWriterLock& operator=(const ReaderWriterLock&) = delete;

    void lockRead();
    void unlockRead();

    void lockWrite();
    void unlockWrite();

    void lockClose();
    void unlockClose();
};

class ReadLockGuard {
private:
    ReaderWriterLock& rwLock;

public:
    explicit ReadLockGuard(ReaderWriterLock& lock);
    ~ReadLockGuard();

    ReadLockGuard(const ReadLockGuard&) = delete;
    ReadLockGuard& operator=(const ReadLockGuard&) = delete;
};

class WriteLockGuard {
private:
    ReaderWriterLock& rwLock;

public:
    explicit WriteLockGuard(ReaderWriterLock& lock);
    ~WriteLockGuard();

    WriteLockGuard(const WriteLockGuard&) = delete;
    WriteLockGuard& operator=(const WriteLockGuard&) = delete;
};

class CloseLockGuard {
private:
    ReaderWriterLock& rwLock;

public:
    explicit CloseLockGuard(ReaderWriterLock& lock);
    ~CloseLockGuard();

    CloseLockGuard(const CloseLockGuard&) = delete;
    CloseLockGuard& operator=(const CloseLockGuard&) = delete;
};