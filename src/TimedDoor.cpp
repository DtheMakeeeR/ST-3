// Copyright 2021 GHA Test Team
#include "TimedDoor.h"
#include <stdexcept>
#include <thread>
#include <chrono>
#include <atomic>
#include <condition_variable>
#include <mutex>

// DoorTimerAdapter implementation
DoorTimerAdapter::DoorTimerAdapter(TimedDoor& door) : door(door) {}

void DoorTimerAdapter::Timeout() {
    door.throwState();
}

// TimedDoor implementation
TimedDoor::TimedDoor(int timeout)
    : iTimeout(timeout), isOpened(false), adapter(nullptr) {
    adapter = new DoorTimerAdapter(*this);
}


bool TimedDoor::isDoorOpened() {
    return isOpened;
}

void TimedDoor::unlock() {
    isOpened = true;
}

void TimedDoor::lock() {
    isOpened = false;
}

int TimedDoor::getTimeOut() {
    return iTimeout;
}

void TimedDoor::throwState() {
    if (isOpened) {
        throw std::runtime_error("Door timeout: door was left open too long!");
    }
}

void Timer::sleep(int seconds) {
    auto start = std::chrono::steady_clock::now();
    auto duration = std::chrono::seconds(seconds);

    while (std::chrono::steady_clock::now() - start < duration) {
        std::this_thread::yield();
    }
}

void Timer::tregister(int timeout, TimerClient* client) {
    if (client == nullptr) {
        return;
    }

    this->client = client;

    if (timeout <= 0) {
        client->Timeout();
        return;
    }

    int waitTimeMs = timeout;

    auto startTime = std::chrono::steady_clock::now();
    auto waitDuration = std::chrono::milliseconds(waitTimeMs);

    while (std::chrono::steady_clock::now() - startTime < waitDuration) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    client->Timeout();
}
