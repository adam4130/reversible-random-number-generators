from numpy.random import default_rng
from prettytable import PrettyTable
from statistics import mean
import timeit

from reverse import (
    UniformRealRNG,
    UniformIntRNG,
    NormalRNG,
    ExponentialRNG,
)


def benchmark(stmt):
    times = timeit.repeat(stmt, number=int(1e6))
    return f"{mean(times):.2f} (\u03BCs)"


def main():
    # Reversible random number generators
    reversible_table = PrettyTable()
    reversible_table.field_names = ["Reversible RNG", "next()", "previous()"]
    for rng in [UniformRealRNG(), UniformIntRNG(),
                NormalRNG(), ExponentialRNG()]:
        next = benchmark(rng.next)
        previous = benchmark(rng.previous)
        reversible_table.add_row([str(rng), next, previous])
    print(reversible_table)

    # NumPy default random number generator
    numpy_rng = default_rng()
    uniform = benchmark(numpy_rng.uniform)
    normal = benchmark(numpy_rng.normal)
    exponential = benchmark(numpy_rng.exponential)

    numpy_table = PrettyTable()
    numpy_table.field_names = [
        "NumPy RNG", "uniform()", "normal()", "exponential()"
    ]
    numpy_table.add_row([str(numpy_rng), uniform, normal, exponential])
    print(numpy_table)


if __name__ == "__main__":
    main()
