/// WARNING: This class is provided for example purposes only. Default to
/// NormalRNG<> in reverse.h which provides a faster and more rigorous
/// implementation of reversible normally distributed random values with the
/// Ziggurat method.

#pragma once

#include <cmath>
#include <limits>
#include <type_traits>
#include <utility>

#include "reverse.h"

namespace reverse {

/// Reversible random number generator on a normal distribution. Uses the
/// Marsaglia polar method to sample normal values from a reversible uniform
/// random source.
template <typename RealType = double>
class ReversiblePolar {
  static_assert(std::is_floating_point<RealType>::value,
      "result_type must be a floating point type");
 public:
  using result_type = RealType;

  ReversiblePolar() : ReversiblePolar(0.0) {}

  explicit ReversiblePolar(result_type mean,
                           result_type stddev = result_type(1.0))
      : mean_(mean), stddev_(stddev), urng_(-1, 1) {
    assert(stddev_ > result_type(0.0));
  }

  void reset() {
    reversing_ = false;
    saved_available_ = false;
  }

  template <typename... Params>
  void seed(Params&&... params) {
    urng_.seed(std::forward<Params>(params)...);
    reset();
  }

  result_type mean() const { return mean_; }
  result_type stddev() const { return stddev_; }

  static constexpr result_type min() { return std::numeric_limits<result_type>::lowest(); }
  static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }

  result_type next();
  result_type previous();
 private:
  template <typename URNG>
  static std::pair<result_type, result_type> polar(URNG& urng);

  bool reversing_ = false, saved_available_ = false;
  result_type saved_ = 0.0, next_saved_ = 0.0;

  const result_type mean_, stddev_;
  UniformRNG<result_type> urng_;
};

template <typename RealType>
  template <typename URNG>
std::pair<typename ReversiblePolar<RealType>::result_type,
          typename ReversiblePolar<RealType>::result_type>
    ReversiblePolar<RealType>::polar(URNG& urng) {
  result_type u, v, s;
  do {
    u = urng();
    v = urng();
    s = u * u + v * v;
  } while (s >= 1.0 || s == 0.0);
  s = std::sqrt(-2 * std::log(s) / s);
  return { u * s, v * s };
}

template <typename RealType>
typename ReversiblePolar<RealType>::result_type
    ReversiblePolar<RealType>::next() {
  if (saved_available_) {
    saved_available_ = false;
    return next_saved_ * stddev() + mean();
  }

  if (reversing_) {
    reversing_ = false;
    urng_.next(2); // Generate past the current saved pair
  }

  // Generate to the next saved pair
  std::tie(saved_, next_saved_) = polar(urng_);
  saved_available_ = true;

  return saved_ * stddev() + mean();
}

template <typename RealType>
typename ReversiblePolar<RealType>::result_type
    ReversiblePolar<RealType>::previous() {
  if (!saved_available_) {
    saved_available_ = true;
    return next_saved_ * stddev() + mean();
  }

  if (!reversing_) {
    reversing_ = true;
    urng_.previous(2); // Reverse past the current saved pair
  }

  const result_type result = saved_ * stddev() + mean();

  // Reverse to the previous saved pair
  ReversedEngine reversed(urng_);
  std::tie(next_saved_, saved_) = polar(reversed);
  saved_available_ = false;

  return result;
}

} // namespace reverse
