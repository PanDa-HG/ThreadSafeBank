#include "ReaderWriterLock.h"

ReaderWriterLock::ReaderWriterLock()
    : activeReaders(0),
      waitingWriters(0),
      waitingClosers(0),
      activeWriter(false) {
}

void ReaderWriterLock::lockRead() {
    std::unique_lock<std::mutex> lock(mtx);

    while (
        activeWriter ||
        waitingWriters > 0 ||
        waitingClosers > 0
    ) {
        cv.wait(lock);
    }

    activeReaders++;
}

void ReaderWriterLock::unlockRead() {
    std::unique_lock<std::mutex> lock(mtx);

    activeReaders--;

    if (activeReaders == 0) {
        cv.notify_all();
    }
}

void ReaderWriterLock::lockWrite() {
    std::unique_lock<std::mutex> lock(mtx);

    waitingWriters++;

    while (
        activeWriter ||
        activeReaders > 0 ||
        waitingClosers > 0
    ) {
        cv.wait(lock);
    }

    waitingWriters--;
    activeWriter = true;
}

void ReaderWriterLock::unlockWrite() {
    std::unique_lock<std::mutex> lock(mtx);

    activeWriter = false;
    cv.notify_all();
}

void ReaderWriterLock::lockClose() {
    std::unique_lock<std::mutex> lock(mtx);

    waitingClosers++;

    while (
        activeWriter ||
        activeReaders > 0
    ) {
        cv.wait(lock);
    }

    waitingClosers--;
    activeWriter = true;
}

void ReaderWriterLock::unlockClose() {
    std::unique_lock<std::mutex> lock(mtx);

    activeWriter = false;
    cv.notify_all();
}

ReadLockGuard::ReadLockGuard(ReaderWriterLock& lock)
    : rwLock(lock) {
    rwLock.lockRead();
}

ReadLockGuard::~ReadLockGuard() {
    rwLock.unlockRead();
}

WriteLockGuard::WriteLockGuard(ReaderWriterLock& lock)
    : rwLock(lock) {
    rwLock.lockWrite();
}

WriteLockGuard::~WriteLockGuard() {
    rwLock.unlockWrite();
}

CloseLockGuard::CloseLockGuard(ReaderWriterLock& lock)
    : rwLock(lock) {
    rwLock.lockClose();
}

CloseLockGuard::~CloseLockGuard() {
    rwLock.unlockClose();
}