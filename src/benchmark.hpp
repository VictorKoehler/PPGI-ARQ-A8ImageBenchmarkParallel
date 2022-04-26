#pragma once
#include <chrono>
#include <string>
#include <unistd.h>

#define CLOCK_PRECISION microseconds
#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)

extern char const *const GIT_COMMIT;

struct BenchClock {
    std::chrono::_V2::system_clock::time_point start;
    BenchClock() : start(std::chrono::high_resolution_clock::now()) {}
    int64_t getElapsed() const {
        const auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::CLOCK_PRECISION>(end - start).count();
    }
};

struct ImagingBenchmark {
    virtual int64_t benchmark(const char *file, bool verbose=false) const = 0;
    virtual const std::string getDesc() const = 0;
};
