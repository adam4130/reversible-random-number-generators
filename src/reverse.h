#pragma once

#include <algorithm>
#include <cstdint>
#include <ios>
#include <istream>
#include <ostream>
#include <random>
#include <tuple>
#include <utility>
#include <vector>

#include "exponential.h"
#include "normal.h"
#include "pcg.h"
#include "uniform.h"

#include "pcg_extras.hpp"

namespace reverse {

/// Wrapper class that changes the direction of a reversible uniform random
/// number generator (RURNG) e.g. ReversiblePCG. Replaces the function-call
/// operator with the RURNG's `previous` function. Conforms to minimum named
/// requirements to be a `UniformRandomBitGenerator`.
template <typename RURNG>
class ReversedEngine {
 public:
  using result_type = typename RURNG::result_type;

  ReversedEngine(RURNG& rurng) : engine_(rurng) {}

  static constexpr result_type min() { return RURNG::min(); }
  static constexpr result_type max() { return RURNG::max(); }

  result_type operator()() { return engine_.previous(); }
 private:
  RURNG& engine_;
};

/// Main templated class for defining a reversible random number generator on a
/// given probability distribution. The underlying reversbile generator is
/// randomly seeded with seed sequence.
template <typename DistType = UniformDistribution<>,
          typename EngineType = ReversiblePCG<>>
class ReversibleRNG {
 public:
  using result_type = typename DistType::result_type;

  template <typename... Args>
  ReversibleRNG(Args&&... args)
      : distribution_(std::forward<Args>(args)...) {
    // Randomly seed from a non-deterministic source if available e.g. /dev/random
    pcg_extras::seed_seq_from<std::random_device> seed_source;
    seed(seed_source);
  }

  template <typename... Args>
  void seed(Args&&... params) {
    engine_.seed(std::forward<Args>(params)...);
    distribution_.reset();
    position_ = 0;
  }

  result_type min() const { return distribution_.min(); }
  result_type max() const { return distribution_.max(); }

  void discard(unsigned long long z) {
    for (; z != 0ULL; --z) {
      (*this)();
    }
  }

  result_type operator()() { return next(); }

  // Returns the next random value
  result_type next() {
    position_++;
    return distribution_(engine_);
  }

  // Returns the previous random value
  result_type previous() {
    position_--;
    ReversedEngine reversed(engine_);
    return distribution_(reversed);
  }

  // Returns a vector of the next random values
  std::vector<result_type> next(std::size_t N) {
    std::vector<result_type> values(N);
    std::generate(values.begin(), values.end(), [&] { return next(); });
    return values;
  }

  // Returns a vector of the previous random values
  std::vector<result_type> previous(std::size_t N) {
    std::vector<result_type> values(N);
    std::generate(values.rbegin(), values.rend(), [&] { return previous(); });
    return values;
  }

  // Returns a tuple of the next random values
  template <std::size_t N>
  auto next() {
    return get(std::make_index_sequence<N>{}, next(N));
  }

  // Returns a tuple of the previous random values
  template <std::size_t N>
  auto previous() {
    return get(std::make_index_sequence<N>{}, previous(N));
  }

  inline std::int64_t position() const { return position_; }

  friend bool operator==(const ReversibleRNG& lhs, const ReversibleRNG& rhs) {
    return lhs.engine_ == rhs.engine_ && lhs.distribution_ == rhs.distribution_
        && lhs.position() == rhs.position();
  }

  friend std::ostream& operator<<(std::ostream& os, const ReversibleRNG& rng) {
    const auto flags = os.flags(std::ios_base::dec | std::ios_base::fixed | std::ios_base::left);
    const auto space = os.widen(' ');
    const auto fill = os.fill(space);

    os << rng.engine_ << space << rng.distribution_ << space << rng.position();

    os.flags(flags);
    os.fill(fill);
    return os;
  }

  friend std::istream& operator>>(std::istream& is, ReversibleRNG& rng) {
    const auto flags = is.flags(std::ios_base::dec | std::ios_base::skipws);

    is >> rng.engine_ >> rng.distribution_ >> rng.position_;

    is.flags(flags);
    return is;
  }
 private:
  template <std::size_t... Is, typename T>
  static auto get(std::index_sequence<Is...>, const std::vector<T>& values) {
    return std::make_tuple(values[Is]...);
  }

  EngineType engine_;
  DistType distribution_;
  std::int64_t position_ = 0;
};

// Convenience type definitions for reversible random number generators on our
// supported probability distributions.

template <typename Numeric = double>
using UniformRNG = ReversibleRNG<UniformDistribution<Numeric>>;

template <typename RealType = double>
using NormalRNG = ReversibleRNG<NormalDistribution<RealType>>;

template <typename RealType = double>
using ExponentialRNG = ReversibleRNG<ExponentialDistribution<RealType>>;

} // namespace reverse
