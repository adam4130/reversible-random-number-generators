import matplotlib.pyplot as plt
import numpy as np
from scipy.stats import norm

from reverse import NormalRNG

rng = NormalRNG()

N = 10000000
x = rng.next(N)

# Verify reversal functionality
assert np.array_equal(x, rng.previous(N))

n_bins = 100
interval = [-5, 5]

counts, edges = np.histogram(x, bins=n_bins, range=interval)
midpoints = np.mean([edges[1:], edges[:-1]], axis=0)
density = norm.pdf(midpoints)
p = (midpoints[1] - midpoints[0]) * density

Np = N * p
chi_squared = np.sum(np.square(counts - Np) / Np)
print(f"Chi-squared = {chi_squared:.2f}")

plt.xlim(interval)
plt.ylabel("Probability density")
plt.hist(midpoints, weights=counts, bins=n_bins, density=True, alpha=0.6, color='g', edgecolor='k', label="Normal PCG")
plt.plot(midpoints, density, 'k', linewidth=2, label="Normal PDF")
plt.legend()
plt.show()
