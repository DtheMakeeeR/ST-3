// Copyright 2021 GHA Test Team
#include "TimedDoor.h"

#include <stdexcept>
#include <thread>
#include <chrono>

DoorTimerAdapter::DoorTimerAdapter(TimedDoor& door) : door(door) {}

void DoorTimerAdapter::Timeout() {
    door.throwState();
}

TimedDoor::TimedDoor(int timeout)
    : iTimeout(timeout), isOpened(false) {
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

int TimedDoor::getTimeOut() const {
    return iTimeout;
}

void TimedDoor::throwState() {
    if (isOpened) {
        throw std::runtime_error("Door is open for too long!");
    }
}

void Timer::sleep(int t) {
    std::this_thread::sleep_for(std::chrono::seconds(t));
}

void Timer::tregister(int timeout, TimerClient* client) {
    this->client = client;
    sleep(timeout);
    client->Timeout();
}
