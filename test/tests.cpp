// Copyright 2021 GHA Test Team

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../include/TimedDoor.h"
#include <stdexcept>
#include <thread>

class MockTimerClient : public TimerClient {
public:
    MOCK_METHOD(void, Timeout, (), (override));
};

class MockDoor : public Door {
public:
    MOCK_METHOD(void, lock, (), (override));
    MOCK_METHOD(void, unlock, (), (override));
    MOCK_METHOD(bool, isDoorOpened, (), (override));
};

class TimedDoorTest : public ::testing::Test {
protected:
    void SetUp() override {
        door = new TimedDoor(100);
    }

    void TearDown() override {
        delete door;
    }

    TimedDoor* door;
};

class TimerTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockClient = new MockTimerClient();
        timer = new Timer();
    }

    void TearDown() override {
        delete timer;
        delete mockClient;
    }

    Timer* timer;
    MockTimerClient* mockClient;
};

TEST_F(TimedDoorTest, InitiallyDoorIsClosed) {
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

TEST_F(TimedDoorTest, GetTimeOutReturnsCorrectValue) {
    EXPECT_EQ(door->getTimeOut(), 100);
}

TEST_F(TimedDoorTest, ThrowStateThrowsExceptionWhenDoorOpened) {
    door->unlock();
    EXPECT_THROW(door->throwState(), std::runtime_error);
}

TEST_F(TimedDoorTest, ThrowStateDoesNotThrowWhenDoorClosed) {
    door->lock();
    EXPECT_NO_THROW(door->throwState());
}

TEST_F(TimedDoorTest, DoorTimerAdapterTimeoutThrowsIfDoorOpen) {
    door->unlock();
    DoorTimerAdapter adapter(*door);
    EXPECT_THROW(adapter.Timeout(), std::runtime_error);
}

TEST_F(TimedDoorTest, DoorTimerAdapterTimeoutDoesNotThrowIfDoorClosed) {
    door->lock();
    DoorTimerAdapter adapter(*door);
    EXPECT_NO_THROW(adapter.Timeout());
}

TEST_F(TimerTest, TimerCallsTimeoutAfterDelay) {
    EXPECT_CALL(*mockClient, Timeout()).Times(1);
    timer->tregister(10, mockClient);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
}

TEST(TimerNullTest, TimerDoesNotCrashWithNullClient) {
    Timer timer;
    EXPECT_NO_THROW(timer.tregister(10, nullptr));
}

TEST(IntegrationTest, FullCycleWithTimeoutThrows) {
    TimedDoor door(50);
    Timer timer;
    DoorTimerAdapter adapter(door);

    door.unlock();
    EXPECT_TRUE(door.isDoorOpened());
    EXPECT_THROW(timer.tregister(50, &adapter), std::runtime_error);
}

TEST(IntegrationTest, NoTimeoutIfDoorClosedBeforeTimeout) {
    TimedDoor door(50);
    Timer timer;
    DoorTimerAdapter adapter(door);

    door.unlock();
    door.lock();
    EXPECT_NO_THROW(timer.tregister(50, &adapter));
}

TEST(TimedDoorConstructorTest, CreatesWithDifferentTimeouts) {
    TimedDoor door1(500);
    TimedDoor door2(1000);

    EXPECT_EQ(door1.getTimeOut(), 500);
    EXPECT_EQ(door2.getTimeOut(), 1000);
}

TEST(DoorTimerAdapterTest, AdapterStoresReferenceToDoor) {
    TimedDoor door(200);
    DoorTimerAdapter adapter(door);

    door.unlock();
    EXPECT_THROW(adapter.Timeout(), std::runtime_error);
}

TEST(TimerTest, TregisterWithZeroTimeout) {
    MockTimerClient client;
    Timer timer;

    EXPECT_CALL(client, Timeout()).Times(1);
    timer.tregister(0, &client);
}

class MockDoorTimerAdapter : public TimerClient {
public:
    MockDoorTimerAdapter(MockDoor& d) : door(d) {}
    void Timeout() override {
        if (door.isDoorOpened()) {
            throw std::runtime_error("Timeout!");
        }
    }
private:
    MockDoor& door;
};

TEST(MockDoorTest, AdapterThrowsWhenMockDoorOpen) {
    MockDoor mockDoor;
    MockDoorTimerAdapter adapter(mockDoor);

    EXPECT_CALL(mockDoor, isDoorOpened())
        .WillOnce(testing::Return(true));

    EXPECT_THROW(adapter.Timeout(), std::runtime_error);
}

TEST(MockDoorTest, AdapterDoesNotThrowWhenMockDoorClosed) {
    MockDoor mockDoor;
    MockDoorTimerAdapter adapter(mockDoor);

    EXPECT_CALL(mockDoor, isDoorOpened())
        .WillOnce(testing::Return(false));

    EXPECT_NO_THROW(adapter.Timeout());
}

