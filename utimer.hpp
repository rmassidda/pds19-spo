#ifndef UTM_HPP
#define UTM_HPP
#include <chrono>
#include <iostream>
#include <string>


class utimer {
  private:
    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point stop;
    std::string message; 

  public:
    utimer(const std::string m) : message(m) {
      start = std::chrono::system_clock::now();
    }

    ~utimer() {
      stop = std::chrono::system_clock::now();
      std::chrono::duration<double> elapsed = stop - start;
      auto musec = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
      std::cout << message << " computed in " << musec << " usec "  << std::endl;
    }
};
#endif
