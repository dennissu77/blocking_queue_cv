#include <iostream>
#include <chrono>



//high_resolution_clock 單位是 奈秒(nanoseconds)
// g++ -std=c++14 -O2 -o check_clock_unit check_clock_unit.cpp
// ./check_clock_unit

int main() {
    using namespace std::chrono;

    if (high_resolution_clock::period::num == 1 && high_resolution_clock::period::den == 1'000'000'000) {
        std::cout << "high_resolution_clock 單位是 奈秒(nanoseconds)" << std::endl;
    } else if (high_resolution_clock::period::num == 1 && high_resolution_clock::period::den == 1'000'000) {
        std::cout << "high_resolution_clock 單位是 微秒(microseconds)" << std::endl;
    } else if (high_resolution_clock::period::num == 1 && high_resolution_clock::period::den == 1'000) {
        std::cout << "high_resolution_clock 單位是 毫秒(milliseconds)" << std::endl;
    } else {
        std::cout << "high_resolution_clock 單位是 1/" << high_resolution_clock::period::den << " 秒" << std::endl;
    }

    return 0;
}
