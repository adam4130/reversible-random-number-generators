#include "battery.h"

#include <cmath>
#include <random>

#include <mersenne.h>
#include <pcg.h>
#include <reverse.h>
#include <xoshiro.h>

using namespace reverse;

/// Wrapper class that outputs uniformly distributed random bits. Converts
/// a normally distributed generator to uniform with the normal CDF.
class NormalCDF : public NormalRNG<> {
 public:
  using result_type = std::uint32_t;

  result_type operator()() {
    const auto normal = NormalRNG<>::operator()();
    return std::erfc(-normal / std::sqrt(2)) / 2 * unif01_NORM32;
  }
};

int main() {
  auto sd = std::random_device{}();
  std::cout << "Seed: " << sd << std::endl;

  Battery<UniformRNG<std::uint32_t>>("Default", sd).BigCrush();
  // Battery<NormalCDF>("Normal CDF", sd).BigCrush();

  // Battery<pcg64>("pcg64", sd).BigCrush();
  // Battery<std::mt19937_64>("mt19937_64", sd).BigCrush();
  // Battery<Xoshiro256>("xoshiro256+", sd).BigCrush();

  // Battery<ReversiblePCG<>>("Reversible PCG", sd).BigCrush();
  // Battery<ReversibleMersenne>("Reversbile Mersenne Twister", sd).BigCrush();
}
