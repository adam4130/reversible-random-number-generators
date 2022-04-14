#pragma once

#include <array>
#include <cstdint>
#include <istream>
#include <limits>
#include <ostream>
#include <type_traits>

namespace reverse {

/// This is a fixed-increment version of Java 8's SplittableRandom generator
/// See http://dx.doi.org/10.1145/2714064.2660195 and
/// http://docs.oracle.com/javase/8/docs/api/java/util/SplittableRandom.html.
/// It is a very fast generator passing BigCrush, and it can be useful if for
/// some reason you absolutely want 64 bits of state.
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

  // This is the jump function for the generator. It is equivalent to 2^128
  // calls to operator() it can be used to generate 2^128 non-overlapping
  // subsequences for parallel computations.
  void jump();

  // This is the long-jump function for the generator. It is equivalent to 2^192
  // calls to operator() it can be used to generate 2^64 starting points, from
  // each of which jump() will generate 2^64 non-overlapping subsequences for
  // parallel distributed computations.
  void long_jump();

  friend bool operator==(const Xoshiro256& lhs, const Xoshiro256& rhs) {
    return lhs.state_ == rhs.state_;
  }

  friend std::ostream& operator<<(std::ostream& os, const Xoshiro256& rng);
  friend std::istream& operator>>(std::istream& is, Xoshiro256& rng);
 private:
  using state_type = std::array<result_type, 4>;
  static constexpr state_type JUMP = { 0x180ec6d33cfd0aba, 0xd5a61266f0c9392c, 0xa9582618e03fc9aa, 0x39abdc4529b1661c };
  static constexpr state_type LONG_JUMP = { 0x76e15d3efefdcbbf, 0xc5004e441c522fb3, 0x77710069854ee241, 0x39109bb02acbe635 };

  state_type state_;
};

template<typename Sseq>
auto Xoshiro256::seed(Sseq& q)
    -> typename std::enable_if<!std::is_convertible<Sseq, result_type>::value>::type {
  // TODO seed state directly ??
  std::array<std::uint32_t, 2> arr;
  q.generate(arr.begin(), arr.end());
  seed(std::uint64_t(arr[1]) << 32 | arr[0]);
}

} /// namespace reverse
