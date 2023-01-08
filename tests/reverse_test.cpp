#include <algorithm>
#include <random>
#include <sstream>
#include <tuple>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>

#include "mersenne.h"
#include "pcg.h"
#include "reverse.h"

#include "pcg_random.hpp"

namespace reverse {

using EngineTypes = std::tuple<
    ReversiblePCG<pcg32>, ReversiblePCG<pcg64>, // Standard PCG configurations
    ReversiblePCG<pcg64_fast>, // LCG increment of 0 which results in slightly reduced 2^126 period
    ReversiblePCG<pcg_engines::cm_setseq_xsl_rr_128_64>, // LCG "cheap" 128-bit multiplier
    ReversibleMersenne>;

using GeneratorTypes = std::tuple<
    ExponentialRNG<float>, ExponentialRNG<double>, NormalRNG<float>, NormalRNG<double>,
    UniformRNG<int>, UniformRNG<long>, UniformRNG<float>, UniformRNG<double>>;

constexpr inline std::size_t N = 1'000'000;

TEMPLATE_LIST_TEST_CASE("Reversible engine can reversed", "[reverse]",
    EngineTypes) {
  TestType g;

  std::vector<typename TestType::result_type> values(N);
  std::generate(values.begin(), values.end(), [&g] { return g.next(); });

  for (auto it = values.rbegin(); it != values.rend(); ++it) {
    REQUIRE(*it == g.previous());
  }
}

TEMPLATE_LIST_TEST_CASE("Reversible engine can be discarded", "[reverse]",
    EngineTypes) {
  TestType g1, g2;
  g1.discard(N);

  for (std::size_t n = 0; n < N; ++n) {
    g2();
  }

  REQUIRE(g1 == g2);
  REQUIRE(g1() == g2());
}

TEMPLATE_LIST_TEST_CASE("Reversible engine can be seeded", "[reverse]",
    EngineTypes) {
  TestType g1, g2;
  g1.discard(N); // Arbitrarily advance the state

  const auto sd = std::random_device{}();
  g1.seed(sd);
  g2.seed(sd);

  REQUIRE(g1 == g2);
  REQUIRE(g1() == g2());
}

TEMPLATE_LIST_TEST_CASE("Reversible engine can be streamed", "[reverse]",
    EngineTypes) {
  TestType g1, g2;
  g1.discard(N); // Arbitrarily advance the state

  std::stringstream ss;
  ss << g1;
  ss >> g2;

  REQUIRE(g1 == g2);
}

TEMPLATE_LIST_TEST_CASE("Reversible engine can be reversed on int type", "[reverse]",
    EngineTypes) {
  const int a = -10;
  const int b = 10;
  ReversibleRNG<UniformDistribution<int>, TestType> rng(a, b);

  std::vector<int> values(N);
  std::generate(values.begin(), values.end(), [&rng] { return rng.next(); });

  for (auto it = values.rbegin(); it != values.rend(); ++it) {
    REQUIRE(*it >= a);
    REQUIRE(*it <= b);
    REQUIRE(*it == rng.previous());
  }
}

TEMPLATE_LIST_TEST_CASE("Reversible engine can be reversed on double type", "[reverse]",
    EngineTypes) {
  const double a = -10.0;
  const double b = 10.0;
  ReversibleRNG<UniformDistribution<double>, TestType> rng(a, b);

  std::vector<double> values(N);
  std::generate(values.begin(), values.end(), [&rng] { return rng.next(); });

  for (auto it = values.rbegin(); it != values.rend(); ++it) {
    REQUIRE(*it >= a);
    REQUIRE(*it < b);
    REQUIRE(*it == rng.previous());
  }
}

TEMPLATE_LIST_TEST_CASE("Reversible RNG can be reversed with vectors", "[reverse]",
    GeneratorTypes) {
  TestType rng;

  auto values = rng.next(N);

  REQUIRE(rng.position() == N);
  REQUIRE(values == rng.previous(N));
  REQUIRE(rng.position() == 0);
}

TEMPLATE_LIST_TEST_CASE("Reversible RNG can be reversed with tuples", "[reverse]",
    GeneratorTypes) {
  TestType rng;

  const std::size_t n = 10; // Typical max tuple size
  auto values = rng.template next<n>();

  REQUIRE(rng.position() == n);
  REQUIRE(values == rng.template previous<n>());
  REQUIRE(rng.position() == 0);
}

TEMPLATE_LIST_TEST_CASE("Reversible RNG can be seeded", "[reverse]",
    GeneratorTypes) {
  TestType rng1, rng2;
  rng1.discard(N); // Arbitrarily advance the state

  const auto sd = std::random_device{}();
  rng1.seed(sd);
  rng2.seed(sd);

  REQUIRE(rng1 == rng2);
}

TEMPLATE_LIST_TEST_CASE("Reversible RNG can be streamed", "[reverse]",
    GeneratorTypes) {
  TestType rng1, rng2;
  rng1.discard(N); // Arbitrarily advance the state

  std::stringstream ss;
  ss << rng1;
  ss >> rng2;

  REQUIRE(rng1 == rng2);
}

TEST_CASE("Reversible 32-bit RNG can be reversed with 64-bit output", "[reverse]") {
  ReversibleRNG<UniformDistribution<std::uint64_t>, ReversiblePCG<pcg32>> rng;

  std::vector<std::uint64_t> values(N);
  std::generate(values.begin(), values.end(), [&rng] { return rng.next(); });

  for (auto it = values.rbegin(); it != values.rend(); ++it) {
    REQUIRE(*it == rng.previous());
  }
}

} // namespace reverse
