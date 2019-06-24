#pragma once

#include <string>
#include <stdexcept>
#include <vector>
#include <chrono>

typedef std::basic_string<uint16_t> String;

#define likely(x)    __builtin_expect(!!(x), 1)
#define unlikely(x)  __builtin_expect(!!(x), 0)
#define NOINLINE __attribute__ ((noinline))

static inline void Die(const std::string& msg) {
  throw std::runtime_error(msg);
}

void ReadLines(const std::string& path, std::vector<std::string>* lines);

/*
 * UTF8 -> uint16_t[]
 * Unsupported/invalid codepoints are replaced with one ' ' per each octet.
 */
void Uconv(const std::string& src, String& dst);

class Timespan {
public:
  Timespan() : ts1(std::chrono::high_resolution_clock::now()) {
  }
  uint64_t microseconds() {
    auto ts2 = std::chrono::high_resolution_clock::now();
    uint64_t delta = std::chrono::duration_cast<std::chrono::microseconds>(ts2 - ts1).count();
    return delta;
  }
private:
  std::chrono::system_clock::time_point ts1;
};

void BenchmarkShingling();
void BenchmarkJaccard();
void JaccardPrintMatches();
