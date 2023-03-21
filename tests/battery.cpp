#include "battery.h"

#include <cmath>
#include <limits>
#include <random>

#include <reverse.h>

using namespace reverse;

/// Wrapper class that outputs uniformly distributed random bits. Converts
/// a normally distributed generator to uniform with the normal CDF. Conforms
/// to the minimum named requirements for a UniformRandomBitGenerator.
class NormalCDF : public NormalRNG<> {
 public:
  using result_type = std::uint32_t;

  static constexpr result_type min() { return std::numeric_limits<result_type>::lowest(); }
  static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }

  result_type operator()() {
    const auto normal = NormalRNG<>::operator()();
    return std::erfc(-normal / std::sqrt(2)) / 2 * unif01_NORM32;
  }
};

int main() {
  auto seed = std::random_device{}();
  std::cout << "Seed: " << seed << std::endl;
  Battery<UniformRNG<std::uint32_t>>(seed).BigCrush();
  // Battery<NormalCDF>(seed).BigCrush();
}
