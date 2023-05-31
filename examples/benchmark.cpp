#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "hash.h"
#include "mersenne.h"
#include "polar.h"
#include "reverse.h"

using namespace reverse;

constexpr std::size_t REPEAT = 5;
constexpr std::size_t NUMBER = 10'000'000;

template<typename F, typename... Args>
static double BenchmarkFunction(F func, Args&&... args) {
  using nanoseconds = std::chrono::nanoseconds;
  using time_point = std::chrono::high_resolution_clock::time_point;

  time_point start = std::chrono::high_resolution_clock::now();
  for (std::size_t i = 0; i < NUMBER; ++i) {
    func(std::forward<Args>(args)...);
  }
  time_point stop = std::chrono::high_resolution_clock::now();

  nanoseconds duration =
    std::chrono::duration_cast<nanoseconds>(stop - start);
  return duration.count() / (double) NUMBER;
}

template <typename RRNG>
static void ReversibleNext(RRNG& rrng) {
  rrng.next();
}

template <typename RRNG>
static void ReversiblePrevious(RRNG& rrng) {
  rrng.previous();
}

template <typename RRNG>
static std::vector<double> BenchmarkReversibleImpl() {
  RRNG rrng(std::random_device{}());
  const auto value = rrng.next();

  const double next = BenchmarkFunction(ReversibleNext<RRNG>, rrng);
  const double previous = BenchmarkFunction(ReversiblePrevious<RRNG>, rrng);

  if (value != rrng.previous()) {
    throw std::runtime_error("RRNG reversal failed.");
  }

  return {next, previous};
}

template <typename DistType, typename URNG>
static void STLRandom(DistType& dist, URNG& urng) {
  dist(urng);
}

template <typename DistType, typename URNG>
static double BenchmarkSTLRandom(URNG& urng) {
  DistType dist;
  return BenchmarkFunction(STLRandom<DistType, URNG>, dist, urng);
}

template <typename URNG>
static std::vector<double> BenchmarkSTLImpl() {
  URNG urng(std::random_device{}());

  const double uniform = BenchmarkSTLRandom<std::uniform_real_distribution<>>(urng);
  const double normal = BenchmarkSTLRandom<std::normal_distribution<>>(urng);
  const double exponential = BenchmarkSTLRandom<std::exponential_distribution<>>(urng);

  return {uniform, normal, exponential};
}

static std::string TimesToCSV(
    const std::vector<double>& times, const std::string& name) {
  std::stringstream ss;
  ss << name << std::fixed << std::setprecision(2);
  for (double time: times) {
    ss << ", " << time << " (ns)";
  }
  return ss.str();
}

template <typename F>
static void BenchmarkImpl(F func, const std::string& name) {
  std::vector<double> sums = func();
  for (auto& s: sums) { s = 0.0; } // Initialize 0's vector
  for (std::size_t i = 0; i < REPEAT; ++i) {
    std::vector<double> times = func();
    std::transform(sums.begin(), sums.end(), times.begin(), sums.begin(),
                   std::plus<double>());
  }
  for (auto& s: sums) { s /= REPEAT; } // Take the average
  std::cout << TimesToCSV(sums, name) << std::endl;
}

/// Prints a CSV representation of the average execution time (in nanoseconds)
/// for `next()` and `previous()` operations on a reversible random number
/// generator (RRNG).
static void BenchmarkReversible() {
  std::cout << "Reversible RNG, next(), previous()" << std::endl;
  BenchmarkImpl(BenchmarkReversibleImpl<UniformRNG<>>, "UniformRNG");
  BenchmarkImpl(BenchmarkReversibleImpl<NormalRNG<>>, "NormalRNG");
  BenchmarkImpl(BenchmarkReversibleImpl<ExponentialRNG<>>,"ExponentialRNG");
  // BenchmarkImpl(BenchmarkReversibleImpl<ReversibleMersenne>, "ReversibleMersenne");
  // BenchmarkImpl(BenchmarkReversibleImpl<ReversiblePolar<>>, "ReversiblePolar");
  // BenchmarkImpl(BenchmarkReversibleImpl<ReversibleHash>, "ReversibleHash");
}

static void BenchmarkSTL() {
  std::cout << "STL RNG, uniform(), normal(), exponential()" << std::endl;
  BenchmarkImpl(BenchmarkSTLImpl<std::minstd_rand>, "minstd_rand"); // LCG
  BenchmarkImpl(BenchmarkSTLImpl<std::mt19937_64>, "mt19937_64"); // Mersenne Twister
  BenchmarkImpl(BenchmarkSTLImpl<std::ranlux48_base>, "ranlux48_base"); // Subtract with carry
}

int main() {
  BenchmarkReversible();
  BenchmarkSTL();
}
