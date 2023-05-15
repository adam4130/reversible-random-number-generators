import timeit
from statistics import mean

from reverse import (
    UniformRealRNG,
    UniformIntRNG,
    NormalRNG,
    ExponentialRNG,
)


def main():
    for rng in [UniformRealRNG(), UniformIntRNG(),
                NormalRNG(), ExponentialRNG()]:
        next = mean(timeit.repeat(rng.next))
        previous = mean(timeit.repeat(rng.previous))
        print(f"{rng}: next = {next:.2f}, previous = {previous:.2f}")


if __name__ == "__main__":
    main()
