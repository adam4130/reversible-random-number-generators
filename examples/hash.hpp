/// WARNING: This class is provided for example purposes only. Default to
/// ReversiblePCG in pcg.h which provides a faster and more rigorous
/// implementation of a revesible uniform bit generator.

#pragma once

#include <array>
#include <cstdint>
#include <ios>
#include <istream>
#include <limits>
#include <ostream>
#include <type_traits>

#include <openssl/sha.h>

namespace reverse {

class ReversibleHash {
 public:
  using result_type = std::uint64_t;

  static constexpr result_type default_seed = 1u;

  explicit ReversibleHash(result_type sd = default_seed) { seed(sd); }

  void seed(result_type sd = default_seed) { state_ = {sd, 0}; }

  template<typename Sseq>
  typename std::enable_if<!std::is_convertible<Sseq, result_type>::value>::type
      seed(Sseq& q);

  static constexpr result_type min() { return std::numeric_limits<result_type>::lowest(); }
  static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }

  void discard(unsigned long long z) { state_.count += z; }

  result_type operator()() { return next(); }

  result_type next() {
    const result_type value = hash();
    state_.count++;
    return value;
  }

  result_type previous() {
    state_.count--;
    return hash();
  }

  friend bool operator==(const ReversibleHash& lhs, const ReversibleHash& rhs) {
    return lhs.state_.seed == rhs.state_.seed && lhs.state_.count == rhs.state_.count;
  }

  friend std::ostream& operator<<(std::ostream& os, const ReversibleHash& rng);
  friend std::istream& operator>>(std::istream& is, ReversibleHash& rng);
 private:
  result_type hash() const;

  struct {
    std::uint64_t seed;
    std::uint64_t count;
  } state_;
};

template<typename Sseq>
auto ReversibleHash::seed(Sseq& q)
    -> typename std::enable_if<!std::is_convertible<Sseq, result_type>::value>::type {
  std::array<std::uint32_t, 2> arr;
  q.generate(arr.begin(), arr.end());
  seed(std::uint64_t(arr[1]) << 32 | arr[0]);
}

ReversibleHash::result_type ReversibleHash::hash() const {
  std::uint8_t hash[SHA256_DIGEST_LENGTH];
  SHA256(reinterpret_cast<const std::uint8_t*>(&state_), sizeof(state_), hash);
  return *reinterpret_cast<result_type*>(hash);
}

std::ostream& operator<<(std::ostream& os, const ReversibleHash& rng) {
  const auto flags = os.flags(std::ios_base::dec | std::ios_base::fixed | std::ios_base::left);
  const auto space = os.widen(' ');
  const auto fill = os.fill(space);

  os << rng.state_.seed << space << rng.state_.count;

  os.flags(flags);
  os.fill(fill);

  return os;
}

std::istream& operator>>(std::istream& is, ReversibleHash& rng) {
  const auto flags = is.flags(std::ios_base::dec | std::ios_base::skipws);

  is >> rng.state_.seed >> rng.state_.count;

  is.flags(flags);
  return is;
}

} // namespace reverse
