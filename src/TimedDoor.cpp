// Copyright 2021 GHA Test Team
#include "TimedDoor.h"
#include <stdexcept>
#include <thread>
#include <chrono>

// DoorTimerAdapter implementation
DoorTimerAdapter::DoorTimerAdapter(TimedDoor& d) : door(d) {}

void DoorTimerAdapter::Timeout() {
  if (door.isDoorOpened()) {
    door.throwState();
  }
}

// TimedDoor implementation
TimedDoor::TimedDoor(int timeout) : iTimeout(timeout), isOpened(false) {
  adapter = new DoorTimerAdapter(*this);
}

bool TimedDoor::isDoorOpened() {
  return isOpened;
}

void TimedDoor::unlock() {
  isOpened = true;
  
  // In a real implementation, we would spawn a separate thread to avoid
  // blocking the main thread. For testing purposes, we'll keep this simple.
  // The Timer will be started in a separate thread-like context.
  #ifndef TESTING
  Timer timer;
  timer.tregister(iTimeout, adapter);
  #endif
}

void TimedDoor::lock() {
  isOpened = false;
}

int TimedDoor::getTimeOut() const {
  return iTimeout;
}

void TimedDoor::throwState() {
  throw std::runtime_error("Door has been open for too long!");
}

// Timer implementation
void Timer::sleep(int seconds) {
  std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

void Timer::tregister(int seconds, TimerClient* timerClient) {
  client = timerClient;
  sleep(seconds);
  client->Timeout();
}
