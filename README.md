# Reversible Random Number Generators

A reversible random number generator library. Allows for reversing sequences of
pseudo-random values on uniform, normal, and exponential probability
distributions. The default underlying engine is the
[PCG](https://www.pcg-random.org/index.html) family of random number generators.
The PCG generators use an LCG to update their internal state. This makes them an
ideal candidate for building a reversible generator. Furthermore, they are
statistically strong (passing TestU01 BigCrush) and very fast. We also provide
two alternative reversible PRNG engines, the Mersenne Twister and a hashing
engine with SHA256. However, the Mersenne Twister has failures within BigCrush,
and the hash generator is about 100 times slower than PCG.

## Build

Conforms to the typical CMake build and test procedure.

```
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build . && ctest
# cmake --install . # Optional step
```

This build process results in a static library (`libReverse.a`) that can used
in conjunction with the public headers. A shared library (`libWrapper.so/dll`)
is also built. It contains a C interface that can be imported from Python with
ctypes. The following CMake will import our reversible library and headers
into your existing project e.g. `main` executable.

```
find_package(Reverse REQUIRED)
add_executable(main main.cpp)
target_link_libraries(main Reverse)
```

### Usage C++

We currently support 3 types of probability distributions with: `UniformRNG`,
`NormalRNG`, and `ExponentialRNG`. Each of these types has a single template
parameter that determines their output type. For the normal and exponential
generators, this template parameter must be a floating point type e.g.
`double`. The constructor determines the underlying distribution parameters
e.g. `UniformRNG<int> rng(-10, 10);` outputs integer values in [-10, 10]. The
generators are automatically seeded with a sequence from `std::random_device`.
There also exist `seed` functions that can be used to set a custom seed or
sequence e.g. `rng.seed(123456789);`.

Minimal example for generating and reversing a sequence of uniformly random
numbers. Values can be generated individually, as vectors, or as tuples with
variations of the `next/previous` functions.

```
#include <vector>

#include <reverse.h> // UniformRNG, NormalRNG, ExponentialRNG

using namespace reverse;

constexpr inline std::size_t N = 1'000'000; // Length of random sequence

int main() {
  // Random number generator on [0.0, 1.0)
  UniformRNG<double> rng; // Equivalent to UniformRNG<double> rng(0.0, 1.0);

  double x = rng.next();
  double y = rng.previous();

  std::vector<double> forward = rng.next(N);
  std::vector<double> backward = rng.previous(N);

  auto  [x1, x2, x3] = rng.next<3>();
  auto  [y1, y2, y3] = rng.previous<3>();
}
```

### Usage Python

Python ctypes allows for the import of a C shared library. Since our reversible
generator is written in C++, we have added a C interface (`libWrapper.so/dll`).
To use the functions contained in this shared library, we include a Python
wrapper for this C wrapper (`python/reverse.py`). Copy or link this module to
the directory of your Python project. The reversible generators can then simply
be accessed with `import reverse`.

**Note:** The `reverse.py` module attempts to automatically find the C wrapper
library (`libWrapper.so/dll`) on your path with the ctypes `find_library`
function. On UNIX systems, the lookup can be aided by with the following.
Alternatively, the constructor of the generators also contains an optional
`path` parameter to specify the complete path to the shared library.

```
$ export LD_LIBRARY_PATH="<Path containing libWrapper.so e.g. /usr/local/lib64>"
OR
>>> rng = UniformRealRNG(path="<Path containing libWrapper.so>/libWrapper.so")
```

Minimal example for generating and reversing a sequence of uniformly random
values. Values can be generated individually or as NumPy arrays with variations
of the `next`/`previous` functions.

```
import numpy as np

from reverse import UniformRealRNG

# Random number generator on [0.0, 1.0)
rng = UniformRealRNG() # Equivalent to UniformRealRNG(0.0, 1.0)

x = rng.next()
y = rng.previous()
assert x == y

N = 1000000 # Length of random sequence
forward = rng.next(N)
backward = rng.previous(N)
assert np.array_equal(forward, backward)
```

## External Dependencies

The default CMake build automatically downloads the PCG library as well as the
Catch2 testing framework. If `BUILD_TESTU01` is enabled, the TestU01 library is
downloaded and built from source (see below). This option requires the Boost
C++ library. The CMake build also has an option, `BUILD_EXAMPLES`, which
generates an executable to benchmark our reversible C++ generators. This option
requires the OpenSSL library (for our reversible cryptographic hash generator).

On Debian based systems, these optional dependencies can be downloaded with the
following.

```
$ sudo apt install libboost-all-dev libssl-dev
```

## TestU01 BigCrush

The TestU01 BigCrush battery has become the standard for checking the statistical
quality of a pseudorandom number generator. We provide a templated C++ wrapper
for the old C-style TestU01 external generator interface (`tests/battery.h`).
Enabling the `BUILD_TESTU01` CMake option will download and build the TestU01
library as well as an executable (`battery`) that randomly seeds our reversible
`UniformRNG` and runs the BigCrush test suite.

```
$ cd build
$ cmake .. -DBUILD_TESTU01=ON
$ cmake --build . && ctest
$ tests/battery > output.txt # Warning: BigCrush takes approximately 4 hours
```

## Performance

The following tables are the output of `examples/benchmark.cpp` and
`python/benchmark.py`.

#### C++ Reversible Random Number Generators

|  Reversible RNG |    next()   |  previous() |
| --------------- | ----------- | ----------- |
|    UniformRNG   |  4.83  (ns) |  4.58  (ns) |
|    NormalRNG    |  11.19 (ns) |  11.99 (ns) |
|  ExponentialRNG |  5.68  (ns) |  6.50  (ns) |

#### Python Reversible Random Number Generators

The Python wrapper of our reversible generators is about 10 times faster than
the NumPy random bit generators.

|            Reversible RNG            |   next()  | previous() |
| ------------------------------------ | --------- | ---------- |
|   UniformRealRNG(a = 0.0, b = 1.0)   | 0.68 (μs) | 0.69 (μs)  |
| UniformIntRNG(a = 0, b = 2147483647) | 0.70 (μs) | 0.72 (μs)  |
|   NormalRNG(mu = 0.0, sigma = 1.0)   | 0.69 (μs) | 0.69 (μs)  |
|     ExponentialRNG(lambd = 1.0)      | 0.73 (μs) | 0.73 (μs)  |


#### NumPy Random Number Generators

|     NumPy RNG      | uniform() |  normal() | exponential() |
|------------------- | --------- | --------- | ------------- |
| Generator(MT19937) | 7.26 (μs) | 4.59 (μs) |   4.31 (μs)   |
|  Generator(PCG64)  | 7.19 (μs) | 4.58 (μs) |   4.27 (μs)   |
| Generator(Philox)  | 7.16 (μs) | 4.56 (μs) |   4.31 (μs)   |
|  Generator(SFC64)  | 7.21 (μs) | 4.55 (μs) |   4.23 (μs)   |
