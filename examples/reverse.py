import ctypes
from ctypes.util import find_library
import numpy as np

# LD_LIBRARY_PATH may need to be set on Linux for ctypes.cdll.LoadLibrary to succeed
# e.g. $ export LD_LIBRARY_PATH="/usr/local/lib64" (https://bugs.python.org/issue18502)
LIB = ctypes.cdll.LoadLibrary(find_library("Wrapper"))

# # Alternatively, load the shared library from the current directory on Windows
# import os
# os.add_dll_directory(os.getcwd())
# os.add_dll_directory(<Path(s) to dependencies of .\\libWrapper.dll e.g. $ ldd libWrapper.dll>)
# LIB = ctypes.cdll.LoadLibrary(".\\libWrapper.dll")

class UniformRealRNG:
    def __init__(self, a=0.0, b=1.0):
        assert a <= b

        self._a = np.float64(a)
        self._b = np.float64(b)

        LIB.uniform_real_create.argtypes = [ctypes.c_double, ctypes.c_double]
        LIB.uniform_real_create.restype = ctypes.c_void_p

        LIB.uniform_real_destroy.argtypes = [ctypes.c_void_p]
        LIB.uniform_real_destroy.restype = None

        LIB.uniform_real_seed.argtypes = [ctypes.c_void_p, ctypes.c_ulonglong]
        LIB.uniform_real_seed.restype = None

        LIB.uniform_real_next.argtypes = [ctypes.c_void_p]
        LIB.uniform_real_next.restype = ctypes.c_double

        LIB.uniform_real_previous.argtypes = [ctypes.c_void_p]
        LIB.uniform_real_previous.restype = ctypes.c_double

        LIB.uniform_real_next_array.argtypes = [ctypes.c_void_p, np.ctypeslib.ndpointer(dtype=np.float64, flags='C_CONTIGUOUS'), ctypes.c_size_t]
        LIB.uniform_real_next_array.restype = None

        LIB.uniform_real_previous_array.argtypes = [ctypes.c_void_p, np.ctypeslib.ndpointer(dtype=np.float64, flags='C_CONTIGUOUS'), ctypes.c_size_t]
        LIB.uniform_real_previous_array.restype = None

        self._rng = LIB.uniform_real_create(self._a, self._b)

    def __del__(self):
        LIB.uniform_real_destroy(self._rng) if hasattr(self, '_rng') else None

    def seed(self, sd):
        LIB.uniform_real_seed(self._rng, sd)

    def next(self, size=1):
        if size == 1:
            return LIB.uniform_real_next(self._rng)

        arr = np.empty(size, dtype=np.float64)
        LIB.uniform_real_next_array(self._rng, arr, size)
        return arr

    def previous(self, size=1):
        if size == 1:
            return LIB.uniform_real_previous(self._rng)

        arr = np.empty(size, dtype=np.float64)
        LIB.uniform_real_previous_array(self._rng, arr, size)
        return arr

    def a(self):
        return self._a

    def b(self):
        return self._b

    def min(self):
        return self.a()

    def max(self):
        return self.b()

class UniformIntRNG:
    def __init__(self, a=0, b=np.iinfo(np.intc).max):
        assert a <= b
        assert a >= np.iinfo(np.intc).min
        assert b <= np.iinfo(np.intc).max

        self._a = np.intc(a)
        self._b = np.intc(b)

        LIB.uniform_int_create.argtypes = [ctypes.c_int, ctypes.c_int]
        LIB.uniform_int_create.restype = ctypes.c_void_p

        LIB.uniform_int_destroy.argtypes = [ctypes.c_void_p]
        LIB.uniform_int_destroy.restype = None

        LIB.uniform_int_seed.argtypes = [ctypes.c_void_p, ctypes.c_ulonglong]
        LIB.uniform_int_seed.restype = None

        LIB.uniform_int_next.argtypes = [ctypes.c_void_p]
        LIB.uniform_int_next.restype = ctypes.c_int

        LIB.uniform_int_previous.argtypes = [ctypes.c_void_p]
        LIB.uniform_int_previous.restype = ctypes.c_int

        LIB.uniform_int_next_array.argtypes = [ctypes.c_void_p, np.ctypeslib.ndpointer(dtype=np.intc, flags='C_CONTIGUOUS'), ctypes.c_size_t]
        LIB.uniform_int_next_array.restype = None

        LIB.uniform_int_previous_array.argtypes = [ctypes.c_void_p, np.ctypeslib.ndpointer(dtype=np.intc, flags='C_CONTIGUOUS'), ctypes.c_size_t]
        LIB.uniform_int_previous_array.restype = None

        self._rng = LIB.uniform_int_create(self._a, self._b)

    def __del__(self):
        LIB.uniform_int_destroy(self._rng) if hasattr(self, '_rng') else None

    def seed(self, sd):
        LIB.uniform_int_seed(self._rng, sd)

    def next(self, size=1):
        if size == 1:
            return LIB.uniform_int_next(self._rng)

        arr = np.empty(size, dtype=np.intc)
        LIB.uniform_int_next_array(self._rng, arr, size)
        return arr

    def previous(self, size=1):
        if size == 1:
            return LIB.uniform_int_previous(self._rng)

        arr = np.empty(size, dtype=np.intc)
        LIB.uniform_int_previous_array(self._rng, arr, size)
        return arr

    def a(self):
        return self._a

    def b(self):
        return self._b

    def min(self):
        return self.a()

    def max(self):
        return self.b()

class NormalRNG:
    def __init__(self, mean=0.0, stddev=1.0):
        assert stddev > 0

        self._mean = np.float64(mean)
        self._stddev = np.float64(stddev)

        LIB.normal_create.argtypes = [ctypes.c_double, ctypes.c_double]
        LIB.normal_create.restype = ctypes.c_void_p

        LIB.normal_destroy.argtypes = [ctypes.c_void_p]
        LIB.normal_destroy.restype = None

        LIB.normal_seed.argtypes = [ctypes.c_void_p, ctypes.c_ulonglong]
        LIB.normal_seed.restype = None

        LIB.normal_next.argtypes = [ctypes.c_void_p]
        LIB.normal_next.restype = ctypes.c_double

        LIB.normal_previous.argtypes = [ctypes.c_void_p]
        LIB.normal_previous.restype = ctypes.c_double

        LIB.normal_next_array.argtypes = [ctypes.c_void_p, np.ctypeslib.ndpointer(dtype=np.float64, flags='C_CONTIGUOUS'), ctypes.c_size_t]
        LIB.normal_next_array.restype = None

        LIB.normal_previous_array.argtypes = [ctypes.c_void_p, np.ctypeslib.ndpointer(dtype=np.float64, flags='C_CONTIGUOUS'), ctypes.c_size_t]
        LIB.normal_previous_array.restype = None

        self._rng = LIB.normal_create(self._mean, self._stddev)

    def __del__(self):
        LIB.normal_destroy(self._rng) if hasattr(self, '_rng') else None

    def seed(self, sd):
        LIB.normal_seed(self._rng, sd)

    def next(self, size=1):
        if size == 1:
            return LIB.normal_next(self._rng)

        arr = np.empty(size, dtype=np.float64)
        LIB.normal_next_array(self._rng, arr, size)
        return arr

    def previous(self, size=1):
        if size == 1:
            return LIB.normal_previous(self._rng)

        arr = np.empty(size, dtype=np.float64)
        LIB.normal_previous_array(self._rng, arr, size)
        return arr

    def mean(self):
        return self._mean

    def stddev(self):
        return self._stddev

    def min(self):
        return np.finfo(np.float64).min

    def max(self):
        return np.finfo(np.float64).max

class ExponentialRNG:
    def __init__(self, lambd=1.0):
        assert lambd > 0

        self._lambd = np.float64(lambd)

        LIB.exponential_create.argtypes = [ctypes.c_double]
        LIB.exponential_create.restype = ctypes.c_void_p

        LIB.exponential_destroy.argtypes = [ctypes.c_void_p]
        LIB.exponential_destroy.restype = None

        LIB.exponential_seed.argtypes = [ctypes.c_void_p, ctypes.c_ulonglong]
        LIB.exponential_seed.restype = None

        LIB.exponential_next.argtypes = [ctypes.c_void_p]
        LIB.exponential_next.restype = ctypes.c_double

        LIB.exponential_previous.argtypes = [ctypes.c_void_p]
        LIB.exponential_previous.restype = ctypes.c_double

        LIB.exponential_next_array.argtypes = [ctypes.c_void_p, np.ctypeslib.ndpointer(dtype=np.float64, flags='C_CONTIGUOUS'), ctypes.c_size_t]
        LIB.exponential_next_array.restype = None

        LIB.exponential_previous_array.argtypes = [ctypes.c_void_p, np.ctypeslib.ndpointer(dtype=np.float64, flags='C_CONTIGUOUS'), ctypes.c_size_t]
        LIB.exponential_previous_array.restype = None

        self._rng = LIB.exponential_create(self._lambd)

    def __del__(self):
        LIB.exponential_destroy(self._rng) if hasattr(self, '_rng') else None

    def seed(self, sd):
        LIB.exponential_seed(self._rng, sd)

    def next(self, size=1):
        if size == 1:
            return LIB.exponential_next(self._rng)

        arr = np.empty(size, dtype=np.float64)
        LIB.exponential_next_array(self._rng, arr, size)
        return arr

    def previous(self, size=1):
        if size == 1:
            return LIB.exponential_previous(self._rng)

        arr = np.empty(size, dtype=np.float64)
        LIB.exponential_previous_array(self._rng, arr, size)
        return arr

    def lambd(self):
        return self._lambd

    def min(self):
        return 0.0

    def max(self):
        return np.finfo(np.float64).max
