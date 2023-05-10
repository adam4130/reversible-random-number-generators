import matplotlib.pyplot as plt
import numpy as np
from scipy.stats import expon

from reverse import ExponentialRNG


def main():
    N = 10000000
    n_bins = 100
    interval = [0, 5]

    rng = ExponentialRNG()
    values = rng.next(N)

    # Verify reversal functionality
    assert np.array_equal(values, rng.previous(N))

    counts, edges = np.histogram(values, bins=n_bins, range=interval)
    midpoints = np.mean([edges[1:], edges[:-1]], axis=0)
    density = expon.pdf(midpoints)
    p = (midpoints[1] - midpoints[0]) * density

    Np = N * p
    chi_squared = np.sum(np.square(counts - Np) / Np)
    print(f"Chi-squared = {chi_squared:.2f}")

    plt.xlim(interval)
    plt.ylabel("Probability density")
    plt.hist(
        midpoints,
        weights=counts,
        bins=n_bins,
        density=True,
        alpha=0.6,
        color="b",
        edgecolor="k",
        label="Exponential PCG",
    )
    plt.plot(midpoints, density, "k", linewidth=2, label="Exponential PDF")
    plt.legend()
    plt.show()


if __name__ == "__main__":
    main()
