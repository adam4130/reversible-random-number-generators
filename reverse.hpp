#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <ios>
#include <istream>
#include <limits>
#include <ostream>
#include <random>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "pcg_random.hpp" // https://github.com/imneme/pcg-cpp

namespace reverse {

#define MULTIPLIER_CONSTANT(type,multiplier,constant)       \
        template <> struct Inverse<type,multiplier<type>> { \
          static constexpr type value = constant;           \
        };

template <typename itype, typename multiplier> struct Inverse {};

// Precomputed multiplicative inverses of the PCG multiplier constants
MULTIPLIER_CONSTANT(std::uint8_t, pcg_detail::default_multiplier, 69U)
MULTIPLIER_CONSTANT(std::uint16_t, pcg_detail::default_multiplier, 8245U)
MULTIPLIER_CONSTANT(std::uint32_t, pcg_detail::default_multiplier,
    3425435293U)
MULTIPLIER_CONSTANT(std::uint64_t, pcg_detail::default_multiplier,
    13877824140714322085ULL)
MULTIPLIER_CONSTANT(pcg_extras::pcg128_t, pcg_detail::default_multiplier,
    PCG_128BIT_CONSTANT(566787436162029664ULL, 11001107174925446285ULL))
MULTIPLIER_CONSTANT(pcg_extras::pcg128_t, pcg_detail::cheap_multiplier,
    PCG_128BIT_CONSTANT(924194304566127212ULL, 10053033838670173597ULL))

/// Wrapper class that implements the reverse of the PCG function-call operator
/// (`previous`). This is done by subclassing a PCG configuration which gives
/// access to the protected internal `state_` and `output` permutation. The PCG
/// random number generator is then reversed by inverting ("unbumping") the LCG
/// transformation on its internal state and reapplying the output function.
template <typename EngineType = pcg64>
class ReversiblePCG : public EngineType {
 public:
  using typename EngineType::result_type;

  // Inherit constructors
  using EngineType::EngineType;

  // Equivalent to `(*this)()`
  result_type next() { return EngineType::operator()(); }

  // Inverse of `next`
  result_type previous() {
    if constexpr (ExtractPCG<EngineType>::output_previous) {
      return EngineType::output(base_ungenerate0());
    }

    return EngineType::output(base_ungenerate());
  }
 protected:
  using typename EngineType::state_type;

  // Inverse of the PCG `engine::bump` state transition function (LCG)
  state_type unbump(state_type state) const {
    constexpr state_type inverse = ExtractPCG<EngineType>::multiplier_inverse;
    return (state - EngineType::increment()) * inverse;
  }

  // Inverse of the PCG `engine::base_generate` function
  state_type base_ungenerate() {
    const state_type old_state = EngineType::state_;
    EngineType::state_ = unbump(old_state);
    return old_state;
  }

  // Inverse of the PCG `engine::base_generate0` function
  state_type base_ungenerate0() {
    return EngineType::state_ = unbump(EngineType::state_);
  }
 private:
  template <typename T>
  struct ExtractPCG;

  /// Gives access to the the `engine` template parameter that determines if
  /// the LCG state value is updated before the output permutation, and the
  /// precomputed multiplier inverses.
  template <typename xtype, typename itype, typename output,
            bool previous, typename stream, typename multiplier>
  struct ExtractPCG<pcg_detail::engine<xtype, itype, output, previous, stream, multiplier>> {
    static constexpr bool output_previous = previous;
    static constexpr itype multiplier_inverse = Inverse<itype, multiplier>::value;
  };
};

namespace util {

template <typename URNG>
inline constexpr typename URNG::result_type range() {
  constexpr typename URNG::result_type min = URNG::min();
  constexpr typename URNG::result_type max = URNG::max();
  static_assert(min < max, "URNG must define min() < max()");
  return max - min;
}

// Lemire's nearly divisionless algorithm, https://arxiv.org/abs/1805.10941.
// Downscales the output of a 64-bit random source to [0, range) without bias.
template <typename URNG>
inline std::uint64_t lemires(URNG& urng, std::uint64_t range) {
  static_assert(util::range<URNG>() == std::numeric_limits<std::uint64_t>::max(),
      "URNG must output 64 bits");
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
// mantissa of a double has 52 bits. Thus, an integer in [0, 2^53) can be
// divided by 2^53 to produce a double precision floating point value in [0, 1)
// without bias. This method is ideal for random number generators with weak low
// bits such as xoshiro256+.
inline double float64(std::uint64_t x) {
  return (x >> 11) * 0x1.0p-53;
}

// Converts the output of a 64-bit random source to a double precision floating
// point value in [0, 1). Guaranteed to only make a single call to the function
// call operator of the given generator.
template <typename URNG>
inline double canonical(URNG& urng) {
  static_assert(range<URNG>() == std::numeric_limits<std::uint64_t>::max(),
      "URNG must output 64 bits");
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
    return uc_type(util::lemires(urng, dist_range + 1)) + a();
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

/// This is a fixed-increment version of Java 8's SplittableRandom generator
/// See http://dx.doi.org/10.1145/2714064.2660195 and
/// http://docs.oracle.com/javase/8/docs/api/java/util/SplittableRandom.html.
/// It is a very fast generator passing BigCrush, and it can be useful if for
/// some reason you absolutely want 64 bits of state such as seeding Xoshiro.
class Splitmix64 {
 public:
  using result_type = std::uint64_t;

  explicit Splitmix64(result_type x) : x_(x) {}

  static constexpr result_type min() { return std::numeric_limits<result_type>::lowest(); }
  static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }

  result_type operator()() {
    result_type z = (x_ += 0x9e3779b97f4a7c15);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
    z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
    return z ^ (z >> 31);
  }
 private:
  result_type x_;
};

/// Adapted from the implementation written in 2018 by David Blackman and
/// Sebastiano Vigna. This is xoshiro256+ 1.0, a generator for floating-point
/// numbers. We suggest to use its upper bits for floating-point generation, as
/// it is slightly faster than xoshiro256++/xoshiro256**. It passes all tests
/// we are aware of except for the lowest three bits, which might fail linearity
/// tests (and just those), so if low linear complexity is not considered an
/// issue (as it is usually the case) it can be used to generate 64-bit outputs,
/// too.
class Xoshiro256 {
 public:
  using result_type = std::uint64_t;

  static constexpr result_type default_seed = 1u;

  explicit Xoshiro256(result_type sd = default_seed) { seed(sd); }

  template<typename Sseq, typename = typename
      std::enable_if<!std::is_convertible<Sseq, result_type>::value>::type>
  explicit Xoshiro256(Sseq& q) { seed(q); }

  void seed(result_type sd = default_seed);

  template<typename Sseq>
  typename std::enable_if<!std::is_convertible<Sseq, result_type>::value>::type
      seed(Sseq& q);

  static constexpr result_type min() { return std::numeric_limits<result_type>::lowest(); }
  static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }

  void discard(unsigned long long z);

  result_type operator()();

  friend bool operator==(const Xoshiro256& lhs, const Xoshiro256& rhs) {
    return lhs.state_ == rhs.state_;
  }

  friend std::ostream& operator<<(std::ostream& os, const Xoshiro256& rng);
  friend std::istream& operator>>(std::istream& is, Xoshiro256& rng);
 private:
  static inline std::uint64_t rotl(std::uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
  }

  std::array<result_type, 4> state_;
};

void Xoshiro256::seed(result_type sd) {
  Splitmix64 rng(sd);
  std::generate(state_.begin(), state_.end(), [&rng] { return rng(); });
}

template<typename Sseq>
auto Xoshiro256::seed(Sseq& q)
    -> typename std::enable_if<!std::is_convertible<Sseq, result_type>::value>::type {
  // TODO seed state directly ??
  std::array<std::uint32_t, 2> arr;
  q.generate(arr.begin(), arr.end());
  seed(std::uint64_t(arr[1]) << 32 | arr[0]);
}

void Xoshiro256::discard(unsigned long long z) {
  for (; z != 0ULL; --z) {
    (*this)();
  }
}

Xoshiro256::result_type Xoshiro256::operator()() {
  const result_type result = state_[0] + state_[3];
  const result_type t = state_[1] << 17;

  state_[2] ^= state_[0];
  state_[3] ^= state_[1];
  state_[1] ^= state_[2];
  state_[0] ^= state_[3];

  state_[2] ^= t;
  state_[3] = rotl(state_[3], 45);

  return result;
}

std::ostream& operator<<(std::ostream& os, const Xoshiro256& rng) {
  const auto flags = os.flags(std::ios_base::dec | std::ios_base::fixed | std::ios_base::left);
  const auto space = os.widen(' ');
  const auto fill = os.fill(space);

  for (const auto& state: rng.state_) {
    os << state << space;
  }

  os.flags(flags);
  os.fill(fill);
  return os;
}

std::istream& operator>>(std::istream& is, Xoshiro256& rng) {
  const auto flags = is.flags(std::ios_base::dec | std::ios_base::skipws);

  for (auto& state: rng.state_) {
    is >> state;
  }

  is.flags(flags);
  return is;
}

template <typename RealType = double>
class NormalDistribution  {
  static_assert(std::is_floating_point<RealType>::value,
      "result_type must be a floating point type");
 public:
  using result_type = RealType;

  NormalDistribution() : NormalDistribution(0.0) {}

  explicit NormalDistribution(result_type mean, result_type stddev = result_type(1.0))
      : mean_(mean), stddev_(stddev) {
    assert(stddev_ > result_type(0.0));
  }

  // Resets the distribution state
  void reset() {}

  // Returns the mean of the distribution
  result_type mean() const { return mean_; }

  // Returns the standard deviation of the distribution
  result_type stddev() const { return stddev_; }

  // Returns the greatest lower bound value of the distribution
  static constexpr result_type min() { return std::numeric_limits<result_type>::lowest(); }

  // Returns the least upper bound value of the distribution
  static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }

  template <typename URNG>
  result_type operator()(URNG& urng) {
    return ziggurat(urng) * stddev() + mean();
  }

  friend bool operator==(const NormalDistribution& lhs, const NormalDistribution& rhs) {
    return lhs.mean() == rhs.mean() && lhs.stddev() == rhs.stddev();
  }

  friend std::ostream& operator<<(std::ostream& os, const NormalDistribution& dist) {
    const auto flags = os.flags(std::ios_base::scientific | std::ios_base::left);
    const auto space = os.widen(' ');
    const auto fill = os.fill(space);
    const auto precision = os.precision(std::numeric_limits<result_type>::max_digits10);

    os << dist.mean() << space << dist.stddev();

    os.flags(flags);
    os.fill(fill);
    os.precision(precision);
    return os;
  }

  friend std::istream& operator>>(std::istream& is, NormalDistribution& dist) {
    const auto flags = is.flags(std::ios_base::dec | std::ios_base::skipws);

    is >> dist.mean_ >> dist.stddev_;

    is.flags(flags);
    return is;
  }
 private:
  template <typename URNG>
  static double ziggurat(URNG& urng);

  static constexpr double R = 3.442619855899;

  static constexpr std::uint32_t KN[] = {
      0x76ad2212, 0x0,        0x600f1b53, 0x6ce447a6, 0x725b46a2, 0x7560051d,
      0x774921eb, 0x789a25bd, 0x799045c3, 0x7a4bce5d, 0x7adf629f, 0x7b5682a6,
      0x7bb8a8c6, 0x7c0ae722, 0x7c50cce7, 0x7c8cec5b, 0x7cc12cd6, 0x7ceefed2,
      0x7d177e0b, 0x7d3b8883, 0x7d5bce6c, 0x7d78dd64, 0x7d932886, 0x7dab0e57,
      0x7dc0dd30, 0x7dd4d688, 0x7de73185, 0x7df81cea, 0x7e07c0a3, 0x7e163efa,
      0x7e23b587, 0x7e303dfd, 0x7e3beec2, 0x7e46db77, 0x7e51155d, 0x7e5aabb3,
      0x7e63abf7, 0x7e6c222c, 0x7e741906, 0x7e7b9a18, 0x7e82adfa, 0x7e895c63,
      0x7e8fac4b, 0x7e95a3fb, 0x7e9b4924, 0x7ea0a0ef, 0x7ea5b00d, 0x7eaa7ac3,
      0x7eaf04f3, 0x7eb3522a, 0x7eb765a5, 0x7ebb4259, 0x7ebeeafd, 0x7ec2620a,
      0x7ec5a9c4, 0x7ec8c441, 0x7ecbb365, 0x7ece78ed, 0x7ed11671, 0x7ed38d62,
      0x7ed5df12, 0x7ed80cb4, 0x7eda175c, 0x7edc0005, 0x7eddc78e, 0x7edf6ebf,
      0x7ee0f647, 0x7ee25ebe, 0x7ee3a8a9, 0x7ee4d473, 0x7ee5e276, 0x7ee6d2f5,
      0x7ee7a620, 0x7ee85c10, 0x7ee8f4cd, 0x7ee97047, 0x7ee9ce59, 0x7eea0eca,
      0x7eea3147, 0x7eea3568, 0x7eea1aab, 0x7ee9e071, 0x7ee98602, 0x7ee90a88,
      0x7ee86d08, 0x7ee7ac6a, 0x7ee6c769, 0x7ee5bc9c, 0x7ee48a67, 0x7ee32efc,
      0x7ee1a857, 0x7edff42f, 0x7ede0ffa, 0x7edbf8d9, 0x7ed9ab94, 0x7ed7248d,
      0x7ed45fae, 0x7ed1585c, 0x7ece095f, 0x7eca6ccb, 0x7ec67be2, 0x7ec22eee,
      0x7ebd7d1a, 0x7eb85c35, 0x7eb2c075, 0x7eac9c20, 0x7ea5df27, 0x7e9e769f,
      0x7e964c16, 0x7e8d44ba, 0x7e834033, 0x7e781728, 0x7e6b9933, 0x7e5d8a1a,
      0x7e4d9ded, 0x7e3b737a, 0x7e268c2f, 0x7e0e3ff5, 0x7df1aa5d, 0x7dcf8c72,
      0x7da61a1e, 0x7d72a0fb, 0x7d30e097, 0x7cd9b4ab, 0x7c600f1a, 0x7ba90bdc,
      0x7a722176, 0x77d664e5 };

  static constexpr double FN[] = {
      1,           0.9635997,   0.9362827,   0.9130436,   0.89228165,  0.87324303,
      0.8555006,   0.8387836,   0.8229072,   0.8077383,   0.793177,    0.7791461,
      0.7655842,   0.7524416,   0.73967725,  0.7272569,   0.7151515,   0.7033361,
      0.69178915,  0.68049186,  0.6694277,   0.658582,    0.6479418,   0.63749546,
      0.6272325,   0.6171434,   0.6072195,   0.5974532,   0.58783704,  0.5783647,
      0.56903,     0.5598274,   0.5507518,   0.54179835,  0.5329627,   0.52424055,
      0.5156282,   0.50712204,  0.49871865,  0.49041483,  0.48220766,  0.4740943,
      0.46607214,  0.4581387,   0.45029163,  0.44252872,  0.43484783,  0.427247,
      0.41972435,  0.41227803,  0.40490642,  0.39760786,  0.3903808,   0.3832238,
      0.37613547,  0.36911446,  0.3621595,   0.35526937,  0.34844297,  0.34167916,
      0.33497685,  0.3283351,   0.3217529,   0.3152294,   0.30876362,  0.30235484,
      0.29600215,  0.28970486,  0.2834622,   0.2772735,   0.27113807,  0.2650553,
      0.25902456,  0.2530453,   0.24711695,  0.241239,    0.23541094,  0.22963232,
      0.2239027,   0.21822165,  0.21258877,  0.20700371,  0.20146611,  0.19597565,
      0.19053204,  0.18513499,  0.17978427,  0.17447963,  0.1692209,   0.16400786,
      0.15884037,  0.15371831,  0.14864157,  0.14361008,  0.13862377,  0.13368265,
      0.12878671,  0.12393598,  0.119130544, 0.11437051,  0.10965602,  0.104987256,
      0.10036444,  0.095787846, 0.0912578,   0.08677467,  0.0823389,   0.077950984,
      0.073611505, 0.06932112,  0.06508058,  0.06089077,  0.056752663, 0.0526674,
      0.048636295, 0.044660863, 0.040742867, 0.03688439,  0.033087887, 0.029356318,
      0.025693292, 0.022103304, 0.018592102, 0.015167298, 0.011839478, 0.008624485,
      0.005548995, 0.0026696292 };

  static constexpr double WN[] = {
      1.7290405e-09, 1.2680929e-10, 1.6897518e-10, 1.9862688e-10, 2.2232431e-10,
      2.4244937e-10, 2.601613e-10,  2.7611988e-10, 2.9073963e-10, 3.042997e-10,
      3.1699796e-10, 3.289802e-10,  3.4035738e-10, 3.5121603e-10, 3.616251e-10,
      3.7164058e-10, 3.8130857e-10, 3.9066758e-10, 3.9975012e-10, 4.08584e-10,
      4.1719309e-10, 4.2559822e-10, 4.338176e-10,  4.418672e-10,  4.497613e-10,
      4.5751258e-10, 4.651324e-10,  4.7263105e-10, 4.8001775e-10, 4.87301e-10,
      4.944885e-10,  5.015873e-10,  5.0860405e-10, 5.155446e-10,  5.2241467e-10,
      5.2921934e-10, 5.359635e-10,  5.426517e-10,  5.4928817e-10, 5.5587696e-10,
      5.624219e-10,  5.6892646e-10, 5.753941e-10,  5.818282e-10,  5.882317e-10,
      5.946077e-10,  6.00959e-10,   6.072884e-10,  6.135985e-10,  6.19892e-10,
      6.2617134e-10, 6.3243905e-10, 6.386974e-10,  6.449488e-10,  6.511956e-10,
      6.5744005e-10, 6.6368433e-10, 6.699307e-10,  6.7618144e-10, 6.824387e-10,
      6.8870465e-10, 6.949815e-10,  7.012715e-10,  7.075768e-10,  7.1389966e-10,
      7.202424e-10,  7.266073e-10,  7.329966e-10,  7.394128e-10,  7.4585826e-10,
      7.5233547e-10, 7.58847e-10,   7.653954e-10,  7.719835e-10,  7.7861395e-10,
      7.852897e-10,  7.920138e-10,  7.987892e-10,  8.0561924e-10, 8.125073e-10,
      8.194569e-10,  8.2647167e-10, 8.3355556e-10, 8.407127e-10,  8.479473e-10,
      8.55264e-10,   8.6266755e-10, 8.7016316e-10, 8.777562e-10,  8.8545243e-10,
      8.932582e-10,  9.0117996e-10, 9.09225e-10,   9.174008e-10,  9.2571584e-10,
      9.341788e-10,  9.427997e-10,  9.515889e-10,  9.605579e-10,  9.697193e-10,
      9.790869e-10,  9.88676e-10,   9.985036e-10,  1.0085882e-09, 1.0189509e-09,
      1.0296151e-09, 1.0406069e-09, 1.0519566e-09, 1.063698e-09,  1.0758702e-09,
      1.0885183e-09, 1.1016947e-09, 1.1154611e-09, 1.1298902e-09, 1.1450696e-09,
      1.1611052e-09, 1.1781276e-09, 1.1962995e-09, 1.2158287e-09, 1.2369856e-09,
      1.2601323e-09, 1.2857697e-09, 1.3146202e-09, 1.347784e-09,  1.3870636e-09,
      1.4357403e-09, 1.5008659e-09, 1.6030948e-09 };

  result_type mean_, stddev_;
};

template <typename RealType>
  template <typename URNG>
double NormalDistribution<RealType>::ziggurat(URNG& urng) {
  static_assert(util::range<URNG>() == std::numeric_limits<std::uint64_t>::max(),
      "URNG must output 64 bits");
  while (true) {
    const std::uint64_t rand = urng();
    const std::uint8_t index = rand & 0x7f; // 127 rectangles
    const std::int32_t r = rand >> 8;

    const double x = r * WN[index];
    if (std::abs(r) < KN[index]) { // 98.78% of the time
      return x;
    }

    // Fast PRNG that can be seeded with 64 bits
    Xoshiro256 rng(rand);

    if (index == 0) {
      double xx, yy;
      do {
        // log1p(-x) = log(1-x) avoids log(0)
        xx = -std::log1p(-util::canonical(rng)) / R;
        yy = -std::log1p(-util::canonical(rng));
      } while (yy + yy < xx * xx);
      return 0 < r ? R + xx : -(R + xx);
    }

    if (FN[index] + util::canonical(rng) * (FN[index-1] - FN[index]) < std::exp(-0.5 * x * x)) {
      return x;
    }
  }
}

template<typename RealType = double>
class ExponentialDistribution {
  static_assert(std::is_floating_point<RealType>::value,
      "result_type must be a floating point type");
 public:
  using result_type = RealType;

  ExponentialDistribution() : ExponentialDistribution(1.0) {}

  explicit ExponentialDistribution(result_type lambda) : lambda_(lambda) {
    assert(lambda_ > result_type(0.0));
  }

  // Resets the distribution state
  void reset() {}

  // Returns the inverse scale parameter of the distribution
  result_type lambda() const { return lambda_; }

  // Returns the greatest lower bound value of the distribution
  result_type min() const { return result_type(0); }

  // Returns the least upper bound value of the distribution
  result_type max() const { return std::numeric_limits<result_type>::max(); }

  template <typename URNG>
  result_type operator()(URNG& urng) {
    static_assert(util::range<URNG>() == std::numeric_limits<std::uint64_t>::max(),
        "URNG must output 64 bits");
	  return -std::log(result_type(1) - util::float64(urng())) / lambda();
  }

  friend bool operator==(const ExponentialDistribution& lhs, const ExponentialDistribution& rhs) {
    return lhs.lambda() == rhs.lambda();
  }

  friend std::ostream& operator<<(std::ostream& os, const ExponentialDistribution& dist) {
    const auto flags = os.flags(std::ios_base::scientific | std::ios_base::left);
    const auto precision = os.precision(std::numeric_limits<result_type>::max_digits10);

    os << dist.lambda();

    os.flags(flags);
    os.precision(precision);
    return os;
  }

  friend std::istream& operator>>(std::istream& is, ExponentialDistribution& dist) {
    const auto flags = is.flags(std::ios_base::dec | std::ios_base::skipws);

    is >> dist.lambda_;

    is.flags(flags);
    return is;
  }
 private:
  result_type lambda_;
};

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
