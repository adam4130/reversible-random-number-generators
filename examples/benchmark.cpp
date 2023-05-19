#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <sstream>
#include <string>
#include <utility>

#include "hash.h"
#include "mersenne.h"
#include "polar.h"
#include "reverse.h"

using namespace reverse;
using nanoseconds_t = std::chrono::nanoseconds;

constexpr inline std::size_t NUMBER = 10'000'000;
constexpr inline std::size_t REPEAT = 5;

/// Returns the average execution time (in nanoseconds) for `next()` and
/// `previous()` operations on a reversible random number generator.
template <typename RRNG>
std::pair<double, double> BenchmarkReversibleRNGImpl(RRNG& rrng) {
  rrng.seed(std::random_device{}());
  const auto value = rrng.next();

  std::chrono::high_resolution_clock::time_point start, stop;
  start = std::chrono::high_resolution_clock::now();
  for (std::size_t i = 0; i < NUMBER; ++i) {
    rrng.next();
  }
  stop = std::chrono::high_resolution_clock::now();
  nanoseconds_t next_duration =
      std::chrono::duration_cast<nanoseconds_t>(stop - start);

  start = std::chrono::high_resolution_clock::now();
  for (std::size_t i = 0; i < NUMBER; ++i) {
    rrng.previous();
  }
  stop = std::chrono::high_resolution_clock::now();
  nanoseconds_t previous_duration =
      std::chrono::duration_cast<nanoseconds_t>(stop - start);

  // Verify reversal
  if (value != rrng.previous()) {
    throw std::runtime_error("Failed to reverse reversible random number generator.");
  }

  return {next_duration.count() / (double) NUMBER,
          previous_duration.count() / (double) NUMBER};
}

/// Prints a CSV representation of the average execution time for a reversible
/// random number generator (RRNG).
template <typename RRNG>
void BenchmarkReversibleRNG(const std::string& name) {
  RRNG rrng;

  double sum_next = 0.0, sum_previous = 0.0;
  for (std::size_t i = 0; i < REPEAT; ++i) {
    auto [next, previous] = BenchmarkReversibleRNGImpl(rrng);
    sum_next += next;
    sum_previous += previous;
  }
  const double avg_next = sum_next / REPEAT;
  const double avg_previous = sum_previous / REPEAT;

  std::stringstream ss;
  ss << std::fixed << std::setprecision(2);
  const std::string precision = " (ns)";

  ss << avg_next;
  const std::string next = ss.str() + precision;

  ss.str(std::string{}); // Empty contents of stream

  ss << avg_previous;
  const std::string previous = ss.str() + precision;

  std::cout << name << ", " << next << ", " << previous << std::endl;
}

int main() {
  BenchmarkReversibleRNG<UniformRNG<>>("UniformRNG");
  BenchmarkReversibleRNG<NormalRNG<>>("NormalRNG");
  BenchmarkReversibleRNG<ExponentialRNG<>>("ExponentialRNG");

  // BenchmarkReversibleRNG<ReversibleHash>("ReversibleHash");
  // BenchmarkReversibleRNG<ReversibleMersenne>("ReversibleMersenne");
  // BenchmarkReversibleRNG<ReversiblePolar<>>("ReversiblePolar");
}