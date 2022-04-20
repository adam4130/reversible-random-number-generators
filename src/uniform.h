#pragma once

#include <cassert>
#include <cstdint>
#include <ios>
#include <istream>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <type_traits>

#include "pcg_extras.hpp"

namespace reverse {

namespace util {

template <typename URNG>
inline constexpr typename URNG::result_type range() {
  constexpr typename URNG::result_type min = URNG::min();
  constexpr typename URNG::result_type max = URNG::max();
  static_assert(min < max, "URNG must define min() < max()");
  return max - min;
}

// Lamire's nearly divisionless algorithm, https://arxiv.org/abs/1805.10941.
// Downscales the output of a 64-bit random source to [0, range) without bias.
template <typename URNG>
inline std::uint64_t lamires(URNG& urng, std::uint64_t range) {
  static_assert(util::range<URNG>() == std::numeric_limits<std::uint64_t>::max(),
      "URNG must output 64-bits");
  using uint128_t = pcg_extras::pcg128_t;

  uint128_t product = uint128_t(urng()) * uint128_t(range);
  std::uint64_t low = product;
  if (low < range) {
    const std::uint64_t threshold = -range % range;
    while (low < threshold) {
      product = uint128_t(urng()) * uint128_t(range);
      low = product;
    }
  }

  return product >> std::numeric_limits<std::uint64_t>::digits;
}

// Uniformly maps a 64-bit integer to the unit interval with its high bits. The
// mantiassa of a double has 52 bits. Thus, an integer in [0, 2^53) can be
// divided by 2^53 to produce a double precision floating point value in [0, 1)
// without bias. This method is ideal for random number generators with weak low
// bits such as xoshiro256+.
inline double float64(std::uint64_t x) {
  return (x >> 11) * 0x1.0p-53;
}

inline float float32(std::uint32_t x) {
  return (x >> 8) * 0x1.0p-24;
}

// Converts the output of a 64-bit random source to a double precision floating
// point value in [0, 1). Guaranteed to only make a single call to the function
// call operator of the given generator.
template <typename URNG>
inline double canonical(URNG& urng) {
  static_assert(range<URNG>() == std::numeric_limits<std::uint64_t>::max(),
      "URNG must output 64-bits");
  return float64(urng());
}

} // namespace util

template <typename IntType = int>
class UniformIntDistribution {
  static_assert(std::is_integral<IntType>::value,
      "result_type must be an integral type");
 public:
  using result_type = IntType;

  UniformIntDistribution() : UniformIntDistribution(0) {}

  explicit UniformIntDistribution(result_type a,
                                  result_type b = std::numeric_limits<result_type>::max())
      : a_(a), b_(b) { assert(a <= b); }

  void reset() {}

  result_type a() const { return a_; }
  result_type b() const { return b_; }

  result_type min() const { return a(); }
  result_type max() const { return b(); }

  template <typename URNG>
  result_type operator()(URNG& urng);

  friend bool operator==(const UniformIntDistribution& lhs, const UniformIntDistribution& rhs) {
    return lhs.a() == rhs.a() && lhs.b() == rhs.b();
  }

  friend std::ostream& operator<<(std::ostream& os, const UniformIntDistribution& dist) {
    const auto flags = os.flags(std::ios_base::scientific | std::ios_base::left);
    const auto space = os.widen(' ');
    const auto fill = os.fill(space);

    os << dist.a() << space << dist.b();

    os.flags(flags);
    os.fill(fill);
    return os;
  }

  friend std::istream& operator>>(std::istream& is, UniformIntDistribution& dist) {
    const auto flags = is.flags(std::ios_base::dec | std::ios_base::skipws);

    is >> dist.a_ >> dist.b_;

    is.flags(flags);
    return is;
  }
 private:
  result_type a_, b_;
};

template <typename IntType>
  template <typename URNG>
typename UniformIntDistribution<IntType>::result_type
    UniformIntDistribution<IntType>::operator()(URNG& urng) {
  using u_type = typename std::make_unsigned<result_type>::type;
  using uc_type = typename std::common_type<typename URNG::result_type, u_type>::type;

  constexpr uc_type urng_range = util::range<URNG>();
  const uc_type dist_range = uc_type(b()) - uc_type(a());

  if (urng_range == dist_range) {
    return uc_type(urng() - urng.min()) + a();
  }

  if (urng_range > dist_range) {
    return uc_type(util::lamires(urng, dist_range + 1)) + a();
  }

  // TODO implement reversible algorithm for urng_range < dist_range
  throw std::runtime_error("Distribution range must be less or equal to URNG range");
}

template <typename RealType = double>
class UniformRealDistribution {
  static_assert(std::is_floating_point<RealType>::value,
      "result_type must be a floating point type");
 public:
  using result_type = RealType;

  UniformRealDistribution() : UniformRealDistribution(0.0) {}

  explicit UniformRealDistribution(result_type a, result_type b = result_type(1.0))
      : a_(a), b_(b) { assert(a <= b); }

  void reset() {}

  result_type a() const { return a_; }
  result_type b() const { return b_; }

  result_type min() const { return a(); }
  result_type max() const { return b(); }

  template <typename URNG>
  result_type operator()(URNG& urng) {
    return util::canonical(urng) * (b() - a()) + a();
  }

  friend bool operator==(const UniformRealDistribution& lhs, const UniformRealDistribution& rhs) {
    return lhs.a() == rhs.a() && lhs.b() == rhs.b();
  }

  friend std::ostream& operator<<(std::ostream& os, const UniformRealDistribution& dist) {
    const auto flags = os.flags(std::ios_base::scientific | std::ios_base::left);
    const auto space = os.widen(' ');
    const auto fill = os.fill(space);
    const auto precision = os.precision(std::numeric_limits<result_type>::max_digits10);

    os << dist.a() << space << dist.b();

    os.flags(flags);
    os.fill(fill);
    os.precision(precision);
    return os;
  }

  friend std::istream& operator>>(std::istream& is, UniformRealDistribution& dist) {
    const auto flags = is.flags(std::ios_base::dec | std::ios_base::skipws);

    is >> dist.a_ >> dist.b_;

    is.flags(flags);
    return is;
  }
 private:
  result_type a_, b_;
};

template <typename Numeric = double>
using UniformDistribution = typename std::conditional<
    std::is_integral<Numeric>::value,
                               UniformIntDistribution<Numeric>,
                               UniformRealDistribution<Numeric>>::type;

} // namespace reverse
