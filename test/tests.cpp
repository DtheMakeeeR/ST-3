// Copyright 2021 GHA Test Team

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdint>
#include <stdexcept>
#include "TimedDoor.h"

class MockTimerClient : public TimerClient {
 public:
    MOCK_METHOD(void, Timeout, (), (override));
};

class TimedDoorTest : public ::testing::Test {
 protected:
    TimedDoor* door;

    void SetUp() override {
        door = new TimedDoor(1);
    }

    void TearDown() override {
        delete door;
    }
};

TEST_F(TimedDoorTest, DoorIsInitiallyClosed) {
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, UnlockOpensDoor) {
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, LockClosesDoor) {
    door->unlock();
    door->lock();
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, ThrowStateThrowsWhenOpen) {
    door->unlock();
    EXPECT_THROW(door->throwState(), std::runtime_error);
}

TEST_F(TimedDoorTest, ThrowStateNoThrowWhenClosed) {
    EXPECT_NO_THROW(door->throwState());
}

TEST_F(TimedDoorTest, GetTimeOutReturnsCorrectValue) {
    EXPECT_EQ(door->getTimeOut(), 1);
}

TEST_F(TimedDoorTest, UnlockThenLockNoThrow) {
    door->unlock();
    door->lock();
    EXPECT_NO_THROW(door->throwState());
}

TEST_F(TimedDoorTest, AdapterTimeoutThrowsWhenOpen) {
    door->unlock();
    DoorTimerAdapter adapter(*door);
    EXPECT_THROW(adapter.Timeout(), std::runtime_error);
}

TEST_F(TimedDoorTest, AdapterTimeoutNoThrowWhenClosed) {
    DoorTimerAdapter adapter(*door);
    EXPECT_NO_THROW(adapter.Timeout());
}

TEST_F(TimedDoorTest, OpenDoorTimerThrow) {
    door->unlock();
    Timer timer;
    DoorTimerAdapter adapter(*door);
    EXPECT_THROW(timer.tregister(door->getTimeOut(), &adapter),
                 std::runtime_error);
}

TEST_F(TimedDoorTest, ClosedDoorTimerNoThrow) {
    Timer timer;
    DoorTimerAdapter adapter(*door);
    EXPECT_NO_THROW(timer.tregister(door->getTimeOut(), &adapter));
}

TEST(TimerMockTest, TimerCallsTimeoutOnClient) {
    MockTimerClient mockClient;
    EXPECT_CALL(mockClient, Timeout()).Times(1);
    Timer timer;
    timer.tregister(1, &mockClient);
}
