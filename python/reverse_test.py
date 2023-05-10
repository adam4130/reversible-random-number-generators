import numpy as np
import pytest
import random

from reverse import UniformRealRNG, UniformIntRNG, NormalRNG, ExponentialRNG


@pytest.mark.parametrize(
    "reversible_rng", [UniformRealRNG, UniformIntRNG, NormalRNG, ExponentialRNG]
)
class TestReverse:
    N = 1000000

    def test_reverse(self, reversible_rng):
        rng = reversible_rng()
        values = [rng.next() for _ in range(self.N)]
        for x in reversed(values):
            assert x == rng.previous()

    def test_reverse_array(self, reversible_rng):
        rng = reversible_rng()
        values = rng.next(self.N)
        assert len(values) == self.N
        assert np.array_equal(values, rng.previous(self.N))

    def test_seed(self, reversible_rng):
        rng1 = reversible_rng()
        rng2 = reversible_rng()

        # Arbitrarily advance one generator
        rng1.next(self.N)

        sd = random.getrandbits(64)
        rng1.seed(sd)
        rng2.seed(sd)

        assert rng1.next() == rng2.next()

    def test_invalid_size(self, reversible_rng):
        rng = reversible_rng()
        with pytest.raises(ValueError):
            rng.next(0)
        with pytest.raises(ValueError):
            rng.previous(0)

    def test_min_max(self, reversible_rng):
        rng = reversible_rng()
        values = rng.next(self.N)
        assert (rng.min() <= values).all()
        assert (rng.max() >= values).all()
