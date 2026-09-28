#include <iostream>
#include "Engine.hpp"

int main() {
    /// Running test
    Trading::Engine engine;
    std::cout << "Testing. 1 + 1 = " 
              << engine.add(1, 1) << std::endl;
    return 0;
}
