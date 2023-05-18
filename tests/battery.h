#pragma once

#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <typeinfo>

#include <boost/core/demangle.hpp>

extern "C" {
#include <bbattery.h>
#include <unif01.h>
}

struct Generator {
  virtual std::uint32_t bits() = 0;
  virtual void write() = 0;
};

extern "C" {

unsigned long bits_dispatch(void*, void* state) {
  return static_cast<Generator*>(state)->bits();
}

double u01_dispatch(void* param, void* state) {
  return bits_dispatch(param, state) / unif01_NORM32;
}

void write_dispatch(void* state) {
  static_cast<Generator*>(state)->write();
}

} // extern "C"

template <typename URNG>
class TestU01Battery : Generator {
  static_assert(std::numeric_limits<typename URNG::result_type>::digits >= 32,
      "result_type must have at least 32 bits");
 public:
  TestU01Battery(std::uint64_t seed = 1u)
      : name_(boost::core::demangle(typeid(URNG).name())) {
    urng_.seed(seed);

    gen_.state = this;
    gen_.param = nullptr;
    gen_.name = const_cast<char*>(name_.c_str());
    gen_.GetU01 = u01_dispatch;
    gen_.GetBits = bits_dispatch;
    gen_.Write = write_dispatch;
  }

  void SmallCrush() {
    bbattery_SmallCrush(&gen_);
  }

  void Crush() {
    bbattery_Crush(&gen_);
  }

  void BigCrush() {
    bbattery_BigCrush(&gen_);
  }

  std::uint32_t bits() override {
    // return urng_() >> 32; // Test low bits of 64-bit generator
    return urng_();
  }

  void write() override {
    std::cout << urng_;
  }
 private:
  const std::string name_;
  unif01_Gen gen_;
  URNG urng_;
};
