// Copyright 2021 GHA Test Team

#ifndef INCLUDE_TIMEDDOOR_H_
#define INCLUDE_TIMEDDOOR_H_

class DoorTimerAdapter;
class Timer;
class Door;
class TimedDoor;

class TimerClient {
 public:
  virtual void Timeout() = 0;
};

class Door {
 public:
  virtual void lock() = 0;
  virtual void unlock() = 0;
  virtual bool isDoorOpened() = 0;
};

class DoorTimerAdapter : public TimerClient {
 private:
  TimedDoor& door;
 public:
  explicit DoorTimerAdapter(TimedDoor&);
  void Timeout();
  
  // Friend classes for testing
  friend class TimedDoorIntegrationTest;
  friend class TimedDoorIntegrationTest_NoExceptionWhenDoorClosed_Test;
};

class TimedDoor : public Door {
 private:
  DoorTimerAdapter* adapter;
  int iTimeout;
  bool isOpened;
 public:
  explicit TimedDoor(int);
  bool isDoorOpened();
  void unlock();
  void lock();
  int getTimeOut() const;
  void throwState();
  
  // For testing purposes
  DoorTimerAdapter* getAdapter() { return adapter; }
  friend class TimedDoorIntegrationTest;
  friend class TimedDoorWithMockTimer;
};

class Timer {
 protected:  // Changed from private to protected
  TimerClient* client;
  virtual void sleep(int);  // Added virtual keyword
 public:
  virtual void tregister(int, TimerClient*);
};

#endif  // INCLUDE_TIMEDDOOR_H_
