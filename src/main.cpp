// Copyright 2021 GHA Test Team
#include "TimedDoor.h"
#include <iostream>
#include <stdexcept>

int main() {
  try {
    std::cout << "Creating a TimedDoor with 5 second timeout..." << std::endl;
    TimedDoor tDoor(5);
    
    std::cout << "Locking the door..." << std::endl;
    tDoor.lock();
    std::cout << "Door is " << (tDoor.isDoorOpened() ? "open" : "closed")
              << std::endl;
    
    std::cout << "Unlocking the door..." << std::endl;
    tDoor.unlock();
    std::cout << "Door is " << (tDoor.isDoorOpened() ? "open" : "closed")
              << std::endl;
    
    std::cout << "The timer will trigger after " << tDoor.getTimeOut()
              << " seconds if the door remains open..." << std::endl;
    
    // In a real scenario, the timer would trigger after the specified timeout
    // and throw an exception if the door was still open.
  } catch (const std::runtime_error& e) {
    std::cout << "Exception caught: " << e.what() << std::endl;
  }

  return 0;
}
