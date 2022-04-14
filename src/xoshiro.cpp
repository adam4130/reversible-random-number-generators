#include "xoshiro.h"

#include <algorithm>
#include <ios>

namespace reverse {

void Xoshiro256::seed(result_type sd) {
  Splitmix64 rng(sd);
  std::generate(state_.begin(), state_.end(), [&rng] { return rng(); });
}

void Xoshiro256::discard(unsigned long long z) {
  for (; z != 0ULL; --z) {
    (*this)();
  }
}

static inline std::uint64_t rotl(uint64_t x, int k) {
  return (x << k) | (x >> (64 - k));
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

void Xoshiro256::jump() {
  result_type s0 = 0;
  result_type s1 = 0;
  result_type s2 = 0;
  result_type s3 = 0;
  for (result_type jump: JUMP) {
    for (int b = 0; b < 64; ++b) {
      if (jump & result_type(1) << b) {
        s0 ^= state_[0];
        s1 ^= state_[1];
        s2 ^= state_[2];
        s3 ^= state_[3];
      }
      operator()();
    }
  }

  state_[0] = s0;
  state_[1] = s1;
  state_[2] = s2;
  state_[3] = s3;
}

void Xoshiro256::long_jump() {
  result_type s0 = 0;
  result_type s1 = 0;
  result_type s2 = 0;
  result_type s3 = 0;
  for (result_type long_jump: LONG_JUMP) {
    for (int b = 0; b < 64; ++b) {
      if (long_jump & result_type(1) << b) {
        s0 ^= state_[0];
        s1 ^= state_[1];
        s2 ^= state_[2];
        s3 ^= state_[3];
      }
      operator()();
    }
  }

  state_[0] = s0;
  state_[1] = s1;
  state_[2] = s2;
  state_[3] = s3;
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

} // namespace reverse
