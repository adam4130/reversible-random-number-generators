import ctypes
from ctypes.util import find_library
from enum import Enum
import numpy as np
import os


class LoadWrapperLibrary(ctypes.CDLL):
    class GeneratorTypes(Enum):
        EXPONENTIAL = 0
        NORMAL = 1
        UNIFORM_INT = 2
        UNIFORM_REAL = 3

        def __str__(self):
            return self.name.lower()

    def __init__(self, path=None):
        if path and not os.path.isfile(path):
            raise ValueError(f"Wrapper library does not exist at {path}.")
        super().__init__(path or find_library("Wrapper"))
        for type in self.GeneratorTypes:
            self._initialize(type)

    def _initialize(self, type):
        is_int = type == self.GeneratorTypes.UNIFORM_INT
        is_exponential = type == self.GeneratorTypes.EXPONENTIAL
        c_type, np_type = ((ctypes.c_int, np.intc) if is_int else
                           (ctypes.c_double, np.float64))

        create = getattr(self, f"{type}_create")
        create.argtypes = [c_type] if is_exponential else [c_type, c_type]
        create.restype = ctypes.c_void_p

        destroy = getattr(self, f"{type}_destroy")
        destroy.argtypes = [ctypes.c_void_p]
        destroy.restype = None

        seed = getattr(self, f"{type}_seed")
        seed.argtypes = [ctypes.c_void_p, ctypes.c_ulonglong]
        seed.restype = None

        next = getattr(self, f"{type}_next")
        next.argtypes = [ctypes.c_void_p]
        next.restype = c_type

        previous = getattr(self, f"{type}_previous")
        previous.argtypes = [ctypes.c_void_p]
        previous.restype = c_type

        next_array = getattr(self, f"{type}_next_array")
        next_array.argtypes = [
            ctypes.c_void_p,
            np.ctypeslib.ndpointer(dtype=np_type, flags="C_CONTIGUOUS"),
            ctypes.c_size_t,
        ]
        next_array.restype = None

        previous_array = getattr(self, f"{type}_previous_array")
        previous_array.argtypes = [
            ctypes.c_void_p,
            np.ctypeslib.ndpointer(dtype=np_type, flags="C_CONTIGUOUS"),
            ctypes.c_size_t,
        ]
        previous_array.restype = None


class UniformRealRNG:
    def __init__(self, a=0.0, b=1.0, path=None):
        if a > b:
            raise ValueError(f"a = {a} must be less than or equal to b = {b}.")

        self._a = np.float64(a)
        self._b = np.float64(b)

        self._lib = LoadWrapperLibrary(path)
        self._rng = self._lib.uniform_real_create(self._a, self._b)

    def __del__(self):
        if hasattr(self, "_rng"):
            self._lib.uniform_real_destroy(self._rng)

    def seed(self, sd):
        self._lib.uniform_real_seed(self._rng, sd)

    def next(self, size=1):
        if size < 1:
            raise ValueError(f"Size = {size} must be greater than 0.")

        if size == 1:
            return self._lib.uniform_real_next(self._rng)

        arr = np.empty(size, dtype=np.float64)
        self._lib.uniform_real_next_array(self._rng, arr, size)
        return arr

    def previous(self, size=1):
        if size < 1:
            raise ValueError(f"Size = {size} must be greater than 0.")

        if size == 1:
            return self._lib.uniform_real_previous(self._rng)

        arr = np.empty(size, dtype=np.float64)
        self._lib.uniform_real_previous_array(self._rng, arr, size)
        return arr

    def a(self):
        return self._a

    def b(self):
        return self._b

    def min(self):
        return self.a()

    def max(self):
        return self.b()

    def __str__(self):
        return f"{self.__class__.__name__}(a = {self.a()}, b = {self.b()})"


class UniformIntRNG:
    def __init__(self, a=0, b=np.iinfo(np.intc).max, path=None):
        if a > b:
            raise ValueError(f"a = {a} must be less than or equal to b = {b}.")
        if a < np.iinfo(np.intc).min or b > np.iinfo(np.intc).max:
            raise ValueError((f"[a = {a}, b = {b}] must be a subset of "
                              f"[{np.iinfo(np.intc).min},"
                              f" {np.iinfo(np.intc).max}]."))

        self._a = np.intc(a)
        self._b = np.intc(b)

        self._lib = LoadWrapperLibrary(path)
        self._rng = self._lib.uniform_int_create(self._a, self._b)

    def __del__(self):
        if hasattr(self, "_rng"):
            self._lib.uniform_int_destroy(self._rng)

    def seed(self, sd):
        self._lib.uniform_int_seed(self._rng, sd)

    def next(self, size=1):
        if size < 1:
            raise ValueError(f"Size = {size} must be greater than 0.")

        if size == 1:
            return self._lib.uniform_int_next(self._rng)

        arr = np.empty(size, dtype=np.intc)
        self._lib.uniform_int_next_array(self._rng, arr, size)
        return arr

    def previous(self, size=1):
        if size < 1:
            raise ValueError(f"Size = {size} must be greater than 0.")

        if size == 1:
            return self._lib.uniform_int_previous(self._rng)

        arr = np.empty(size, dtype=np.intc)
        self._lib.uniform_int_previous_array(self._rng, arr, size)
        return arr

    def a(self):
        return self._a

    def b(self):
        return self._b

    def min(self):
        return self.a()

    def max(self):
        return self.b()

    def __str__(self):
        return f"{self.__class__.__name__}(a = {self.a()}, b = {self.b()})"


class NormalRNG:
    def __init__(self, mu=0.0, sigma=1.0, path=None):
        if sigma <= 0:
            raise ValueError(f"Sigma = {sigma} must be greater than 0.")

        self._mu = np.float64(mu)
        self._sigma = np.float64(sigma)

        self._lib = LoadWrapperLibrary(path)
        self._rng = self._lib.normal_create(self._mu, self._sigma)

    def __del__(self):
        if hasattr(self, "_rng"):
            self._lib.normal_destroy(self._rng)

    def seed(self, sd):
        self._lib.normal_seed(self._rng, sd)

    def next(self, size=1):
        if size < 1:
            raise ValueError(f"Size = {size} must be greater than 0.")

        if size == 1:
            return self._lib.normal_next(self._rng)

        arr = np.empty(size, dtype=np.float64)
        self._lib.normal_next_array(self._rng, arr, size)
        return arr

    def previous(self, size=1):
        if size < 1:
            raise ValueError(f"Size = {size} must be greater than 0.")

        if size == 1:
            return self._lib.normal_previous(self._rng)

        arr = np.empty(size, dtype=np.float64)
        self._lib.normal_previous_array(self._rng, arr, size)
        return arr

    def mu(self):
        return self._mu

    def sigma(self):
        return self._sigma

    def min(self):
        return np.finfo(np.float64).min

    def max(self):
        return np.finfo(np.float64).max

    def __str__(self):
        return (f"{self.__class__.__name__}"
                f"(mu = {self.mu()}, sigma = {self.sigma()})")


class ExponentialRNG:
    def __init__(self, lambd=1.0, path=None):
        if lambd <= 0:
            raise ValueError(f"Lambd = {lambd} must be greater than 0.")

        self._lambd = np.float64(lambd)

        self._lib = LoadWrapperLibrary(path)
        self._rng = self._lib.exponential_create(self._lambd)

    def __del__(self):
        if hasattr(self, "_rng"):
            self._lib.exponential_destroy(self._rng)

    def seed(self, sd):
        self._lib.exponential_seed(self._rng, sd)

    def next(self, size=1):
        if size < 1:
            raise ValueError(f"Size = {size} must be greater than 0.")

        if size == 1:
            return self._lib.exponential_next(self._rng)

        arr = np.empty(size, dtype=np.float64)
        self._lib.exponential_next_array(self._rng, arr, size)
        return arr

    def previous(self, size=1):
        if size < 1:
            raise ValueError(f"Size = {size} must be greater than 0.")

        if size == 1:
            return self._lib.exponential_previous(self._rng)

        arr = np.empty(size, dtype=np.float64)
        self._lib.exponential_previous_array(self._rng, arr, size)
        return arr

    def lambd(self):
        return self._lambd

    def min(self):
        return 0.0

    def max(self):
        return np.finfo(np.float64).max

    def __str__(self):
        return (f"{self.__class__.__name__}(lambd = {self.lambd()})")
