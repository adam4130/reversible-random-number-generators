#!/usr/bin/env python3

from numpy.random import Generator, MT19937, PCG64, Philox, SFC64
from prettytable import PrettyTable
from statistics import mean
import sys
import timeit

from reverse import UniformRealRNG, UniformIntRNG, NormalRNG, ExponentialRNG


def benchmark(stmt):
    times = timeit.repeat(stmt, number=int(1e6))
    return f"{mean(times):.2f} (\u03BCs)"


def benchmark_reversible():
    table = PrettyTable()
    table.title = "Python Reversible Random Number Generators"
    table.field_names = ["Reversible RNG", "next()", "previous()"]
    for rng in [UniformRealRNG(), UniformIntRNG(),
                NormalRNG(), ExponentialRNG()]:
        next = benchmark(rng.next)
        previous = benchmark(rng.previous)
        table.add_row([str(rng), next, previous])
    print(table)


def benchmark_numpy():
    table = PrettyTable()
    table.title = "NumPy Random Number Generators"
    table.field_names = ["NumPy RNG", "uniform()", "normal()", "exponential()"]
    for type in [MT19937, PCG64, Philox, SFC64]:
        rng = Generator(type())
        uniform = benchmark(rng.uniform)
        normal = benchmark(rng.normal)
        exponential = benchmark(rng.exponential)
        table.add_row([str(rng), uniform, normal, exponential])
    print(table)


def print_table():
    table = PrettyTable()
    table.title = "C++ Reversible Random Number Generators"
    table.field_names = ["Reversible RNG", "next()", "previous()"]
    for line in sys.stdin:
        table.add_row([x.strip() for x in line.split(",")])
    print(table)


def main():
    benchmark_reversible()
    # benchmark_numpy()
    # print_table()


if __name__ == "__main__":
    main()
