#pragma once

#include <cassert>
#include <cmath>
#include <ios>
#include <istream>
#include <limits>
#include <ostream>
#include <type_traits>

#include "uniform.h"

namespace reverse {

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
        "URNG must output 64-bits");
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

} // namespace reverse

