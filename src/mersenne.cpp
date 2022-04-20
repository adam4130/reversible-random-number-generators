#include "mersenne.h"

#include <ios>

namespace reverse {

void ReversibleMersenne::seed(result_type sd) {
  state_[0] = sd;
  for (std::size_t i = 1; i < state_size; ++i) {
    result_type x = state_[i - 1];
    x ^= x >> (word_size - 2);
    x *= initialization_multiplier;
    x += i;
    state_[i] = x;
  }

  pos_ = state_size;
}

void ReversibleMersenne::discard(unsigned long long z) {
  while (z > state_size - pos_) {
    z -= state_size - pos_;
    twist();
  }

  pos_ += z;
}

ReversibleMersenne::result_type ReversibleMersenne::next() {
  if (pos_ >= int(state_size)) {
    twist();
  }

  return temper(state_[pos_++]);
}

ReversibleMersenne::result_type ReversibleMersenne::previous() {
  if (pos_ <= 0) {
    untwist();
  }

  return temper(state_[--pos_]);
}

std::ostream& operator<<(std::ostream& os, const ReversibleMersenne& rng) {
  const auto flags = os.flags(std::ios_base::dec | std::ios_base::fixed | std::ios_base::left);
  const auto space = os.widen(' ');
  const auto fill = os.fill(space);

  for (const auto& state: rng.state_) {
    os << state << space;
  }
  os << rng.pos_;

  os.flags(flags);
  os.fill(fill);
  return os;
}

std::istream& operator>>(std::istream& is, ReversibleMersenne& rng) {
  const auto flags = is.flags(std::ios_base::dec | std::ios_base::skipws);

  for (auto& state: rng.state_) {
    is >> state;
  }
  is >> rng.pos_;

  is.flags(flags);
  return is;
}

void ReversibleMersenne::twist() {
  for (std::size_t k = 0; k < state_size; ++k) {
    result_type y = ((state_[k] & upper_mask) | (state_[(k + 1) % state_size] & lower_mask));
    state_[k] = state_[(k + shift_size) % state_size] ^ (y >> 1) ^ ((y & 0x01) ? xor_mask : 0);
  }

  pos_ = 0;
}

// https://jazzy.id.au/2010/09/25/cracking_random_number_generators_part_4.html
void ReversibleMersenne::untwist() {
  for (int k = state_size - 1; k >= 0; --k) {
    result_type y = state_[k] ^ state_[(k + shift_size) % state_size];
    y ^= (y & first_mask) ? xor_mask : 0;
    state_[k] = (y << 1) & upper_mask;

    y = state_[(k - 1 + state_size) % state_size] ^ state_[(k - 1 + shift_size) % state_size];
    if (y & first_mask) {
      y ^= xor_mask;
      state_[k] |= 0x01;
    }
    state_[k] |= (y << 1) & lower_mask;
  }

  pos_ = state_size;
}

ReversibleMersenne::result_type ReversibleMersenne::temper(result_type z) {
  z ^= (z >> tempering_u) & tempering_d;
  z ^= (z << tempering_s) & tempering_b;
  z ^= (z << tempering_t) & tempering_c;
  z ^= (z >> tempering_l);
  return z;
}

} // namespace reverse
