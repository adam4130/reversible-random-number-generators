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

Conforms to the modern CMake build and test procedure.

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

The CMake build automatically pulls in the PCG library as well as the Catch2
testing framework (and optionally TestU01). It also constructs the Python
wrapper C shared library for ctypes.

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
generator is written in C++, we added a C interface (`libWrapper.so/dll`). To
use the functions contained in this shared library we include a Python wrapper for
this C wrapper (`python/reverse.py`). Copy or link this module to the directory
of your Python project. The reversible generators can then simply be accessed with
`import reverse`.

**Note:** The `reverse.py` module attempts to automatically find the C wrapper
library (`libWrapper.so/dll`) on your path. However, the ctypes `find_library`
function is very fragile. On UNIX systems, the lookup can be aided by with the
following.

```
$ export LD_LIBRARY_PATH="<Path containing libWrapper.so e.g. /usr/local/lib64>"
```

Minimal example for generating and reversing a sequence of uniformly random values.
Values can be generated individually or as NumPy arrays with variations of the
`next/previous` functions.

```
import numpy as np

# import reverse
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

## TestU01 BigCrush

The TestU01 BigCrush battery has become the standard for checking the statistical
quality of a pseudorandom number generator. We provide a C++ wrapper to interface
with the old C-style TestU01 external generators. Enable the `BUILD_TESTU01` CMake
option to download and build the TestU01 library. It also creates an executable
(`battery`) that randomly seeds our reversible `UniformRNG` and runs the BigCrush
battery.

```
$ cd build
$ cmake .. -DBUILD_TESTU01=ON
$ cmake --build . && ctest
$ tests/battery > results.txt # Warning: BigCrush takes approximately 4 hours
```

## Performance

The following tables are the output of `python/benchmark.py`. They demonstrate
that our reversible generators and Python wrapper are about 10 times faster than
the NumPy `default_rng` implementation (which also happens to be PCG).


```
+--------------------------------------+-----------+------------+
|            Reversible RNG            |   next()  | previous() |
+--------------------------------------+-----------+------------+
|   UniformRealRNG(a = 0.0, b = 1.0)   | 0.67 (μs) | 0.67 (μs)  |
| UniformIntRNG(a = 0, b = 2147483647) | 0.72 (μs) | 0.72 (μs)  |
|   NormalRNG(mu = 0.0, sigma = 1.0)   | 0.69 (μs) | 0.69 (μs)  |
|     ExponentialRNG(lambd = 1.0)      | 0.75 (μs) | 0.74 (μs)  |
+--------------------------------------+-----------+------------+
```

```
+------------------+-----------+-----------+---------------+
|    NumPy RNG     | uniform() |  normal() | exponential() |
+------------------+-----------+-----------+---------------+
| Generator(PCG64) | 7.03 (μs) | 4.36 (μs) |   4.14 (μs)   |
+------------------+-----------+-----------+---------------+
```
