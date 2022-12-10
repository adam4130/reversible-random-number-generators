#pragma once

#include <array>
#include <cstdint>
#include <istream>
#include <limits>
#include <ostream>
#include <type_traits>

namespace reverse {

class ReversibleMersenne {
 public:
  using result_type = std::uint64_t;

  static constexpr result_type default_seed = 5489u;

  explicit ReversibleMersenne(result_type sd = default_seed) { seed(sd); }

  template<typename Sseq, typename = typename
      std::enable_if<!std::is_convertible<Sseq, result_type>::value>::type>
  explicit ReversibleMersenne(Sseq& q) { seed(q); }

  void seed(result_type sd = default_seed);

  template<typename Sseq>
  typename std::enable_if<!std::is_convertible<Sseq, result_type>::value>::type
      seed(Sseq& q);

  static constexpr result_type min() { return std::numeric_limits<result_type>::lowest(); }
  static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }

  void discard(unsigned long long z);

  result_type operator()() { return next(); }

  result_type next();
  result_type previous();

  friend bool operator==(const ReversibleMersenne& lhs, const ReversibleMersenne& rhs) {
    return lhs.state_ == rhs.state_ && lhs.pos_ == rhs.pos_;
  }

  friend std::ostream& operator<<(std::ostream& os, const ReversibleMersenne& rng);
  friend std::istream& operator>>(std::istream& is, ReversibleMersenne& rng);
 private:
  void twist();
  void untwist();

  static result_type temper(result_type z);

  static constexpr std::size_t word_size = 64;
  static constexpr std::size_t state_size = 312;
  static constexpr std::size_t shift_size = 156;
  static constexpr result_type upper_mask = (~result_type()) << 31;
  static constexpr result_type lower_mask = ~upper_mask;
  static constexpr result_type xor_mask = 0xb5026f5aa96619e9ULL;
  static constexpr result_type first_mask = 0x8000000000000000ULL; // 2^63
  static constexpr std::size_t tempering_u = 29;
  static constexpr result_type tempering_d = 0x5555555555555555ULL;
  static constexpr std::size_t tempering_s = 17;
  static constexpr result_type tempering_b = 0x71d67fffeda60000ULL;
  static constexpr std::size_t tempering_t = 37;
  static constexpr result_type tempering_c = 0xfff7eee000000000ULL;
  static constexpr std::size_t tempering_l = 43;
  static constexpr result_type initialization_multiplier = 6364136223846793005ULL;

  int pos_ = 0;
  std::array<result_type, state_size> state_;
};

template<typename Sseq>
auto ReversibleMersenne::seed(Sseq& q)
    -> typename std::enable_if<!std::is_convertible<Sseq, result_type>::value>::type {
  std::array<std::uint32_t, 2> arr;
  q.generate(arr.begin(), arr.end());
  seed(std::uint64_t(arr[1]) << 32 | arr[0]);
}

} // namespace reverse
