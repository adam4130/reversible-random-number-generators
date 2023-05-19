#include "battery.h"

#include <cmath>
#include <limits>
#include <random>

#include "reverse.h"

using namespace reverse;

/// Wrapper class that converts our normally distributed random number
/// generator to a uniformly distributed generator. This allows for testing
/// with uniform batteries e.g. TestU01 BigCrush. Conversion is done with the
/// normal CDF function.
class NormalCDF : NormalRNG<> {
 public:
  using result_type = std::uint32_t;

  static constexpr result_type min() { return std::numeric_limits<result_type>::lowest(); }
  static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }

  result_type operator()() {
    const auto normal = NormalRNG<>::operator()();
    return std::erfc(-normal / std::sqrt(2)) / 2 * unif01_NORM32;
  }

  // Inherit necessary functions for TestU01Battery
  using NormalRNG<>::seed;

  friend std::ostream& operator<<(std::ostream& os, const NormalCDF& rng) {
    return os << static_cast<const NormalRNG<>&>(rng);
  }
};

int main() {
  auto seed = std::random_device{}();
  std::cout << "Seed: " << seed << std::endl;
  TestU01Battery<UniformRNG<std::uint64_t>>(seed).BigCrush();
  // TestU01Battery<NormalCDF>(seed).BigCrush();
}
