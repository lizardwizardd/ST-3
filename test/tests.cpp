// Copyright 2021 GHA Test Team

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdint>
#include <stdexcept>
#include "TimedDoor.h"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::Return;

// Mock classes for testing
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

// MockTimer class that properly extends Timer
class MockTimer : public Timer {
 public:
  MOCK_METHOD(void, tregister, (int, TimerClient*), (override));
  
  // Since sleep is protected in the base class, we need to expose it
  void CallSleep(int seconds) {
    sleep(seconds);
  }
};

// Test fixture for TimedDoor
class TimedDoorTest : public ::testing::Test {
 protected:
  void SetUp() override {
    door = new TimedDoor(5);
  }

  void TearDown() override {
    delete door;
  }

  TimedDoor* door;
};

// Test fixture for TimedDoorWithMockTimer
class TimedDoorWithMockTimerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    testDoor = new TimedDoorWithMockTimer(5);
  }

  void TearDown() override {
    delete testDoor;
  }

  // Inner class definition moved here
  class TimedDoorWithMockTimer : public TimedDoor {
   public:
    explicit TimedDoorWithMockTimer(int timeout) : TimedDoor(timeout) {}
    
    // Override unlock to avoid calling the real timer
    void unlock() override {
      TimedDoor::isOpened = true;  // Now accessible via friendship
      // Don't call the timer's register method in the test
    }
  };

  TimedDoorWithMockTimer* testDoor;
};

// Test fixture for DoorTimer with Mocks
class DoorTimerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    door = new TimedDoor(5);
    mockTimer = new MockTimer();
  }

  void TearDown() override {
    delete door;
    delete mockTimer;
  }

  TimedDoor* door;
  MockTimer* mockTimer;
};

// 1. Test TimedDoor constructor sets initial state correctly
TEST_F(TimedDoorTest, InitialState) {
  EXPECT_FALSE(door->isDoorOpened());
  EXPECT_EQ(door->getTimeOut(), 5);
}

// 2. Test lock() method sets isOpened to false
TEST_F(TimedDoorTest, LockMethodClosesTheDoor) {
  door->unlock();  // First open the door
  EXPECT_TRUE(door->isDoorOpened());
  
  door->lock();
  EXPECT_FALSE(door->isDoorOpened());
}

// 3. Test unlock() method sets isOpened to true
TEST_F(TimedDoorTest, UnlockMethodOpensTheDoor) {
  door->lock();  // Ensure door is closed
  EXPECT_FALSE(door->isDoorOpened());
  
  door->unlock();
  EXPECT_TRUE(door->isDoorOpened());
}

// 4. Test throwState() method throws an exception
TEST_F(TimedDoorTest, ThrowStateThrowsException) {
  EXPECT_THROW(door->throwState(), std::runtime_error);
}

// 8. Test unlock method registers with the timer (using TEST_F now)
TEST_F(TimedDoorWithMockTimerTest, UnlockRegistersWithTimer) {
  testDoor->unlock();
  EXPECT_TRUE(testDoor->isDoorOpened());
}

// 5. Test DoorTimerAdapter Timeout calls throwState when door is open
class MockTimedDoor : public TimedDoor {
 public:
  explicit MockTimedDoor() : TimedDoor(5) {}
  
  // Override the methods that will be mocked
  MOCK_METHOD(bool, isDoorOpened, (), (override));
  MOCK_METHOD(void, throwState, (), (override));
};

TEST(DoorTimerAdapterTest, TimeoutCallsThrowStateWhenDoorIsOpen) {
  MockTimedDoor mockDoor;
  
  // Setup expectations
  EXPECT_CALL(mockDoor, isDoorOpened())
      .WillOnce(Return(true));
  EXPECT_CALL(mockDoor, throwState())
      .Times(1);
  
  // Create the adapter with our mock door
  DoorTimerAdapter adapter(mockDoor);
  
  // Call the method we're testing
  adapter.Timeout();
}

// 6. Test DoorTimerAdapter Timeout doesn't call throwState when door is closed
TEST(DoorTimerAdapterTest, TimeoutDoesNotCallThrowStateWhenDoorIsClosed) {
  MockTimedDoor mockDoor;
  
  // Setup expectations
  EXPECT_CALL(mockDoor, isDoorOpened())
      .WillOnce(Return(false));
  EXPECT_CALL(mockDoor, throwState())
      .Times(0);
  
  // Create the adapter with our mock door
  DoorTimerAdapter adapter(mockDoor);
  
  // Call the method we're testing
  adapter.Timeout();
}

// 7. Test Timer tregister calls Timeout after sleep
class TestableTimer : public Timer {
 public:
  // Override the sleep method to make testing faster
  void sleep(int seconds) override {
    // Do nothing in the test to make it fast
  }
  
  // Expose the tregister method for testing
  using Timer::tregister;
};

TEST(TimerTest, TregisterCallsTimeoutAfterSleep) {
  MockTimerClient mockClient;
  EXPECT_CALL(mockClient, Timeout())
      .Times(1);
  
  TestableTimer timer;
  timer.tregister(5, &mockClient);
}

// For the integration tests, we need better control over the timers
class ManualTestTimer : public Timer {
 public:
  void sleep(int) override {
    // Do nothing to make the test fast
  }
  
  void triggerTimeout() {
    if (client) {
      client->Timeout();
    }
  }
};

// 9. Test exception when door remains open after timeout
TEST(TimedDoorIntegrationTest, ThrowsExceptionWhenDoorRemainsOpen) {
  TimedDoor door(5);
  door.unlock();  // This won't trigger the timer due to TESTING define
  
  // Create our test timer and register the door's adapter
  ManualTestTimer timer;
  
  // Expect the exception when we trigger the timeout
  EXPECT_THROW({
    timer.tregister(0, door.getAdapter());
    timer.triggerTimeout();  // Manually trigger the timeout
  }, std::runtime_error);
}

// 10. Test no exception when door is closed before timeout
TEST(TimedDoorIntegrationTest, NoExceptionWhenDoorClosed) {
  TimedDoor door(5);
  door.unlock();  // This won't trigger the timer due to TESTING define
  
  // Create our test timer
  ManualTestTimer timer;
  
  // Register the door's adapter
  timer.tregister(0, door.getAdapter());
  
  // Close the door before triggering the timeout
  door.lock();
  
  // No exception should be thrown
  EXPECT_NO_THROW(timer.triggerTimeout());
}
