import numpy as np


def make_sin_signal(shape, period, amplitude):
    h, w = shape
    xs = np.arange(w) * period
    ys = (np.sin(xs) + 1.0) * amplitude / 2
    return np.tile(ys, (h, 1))


def make_signal1(shape, amplitude):
    h, w = shape
    xs = np.arange(w) * 0.03
    a = np.tile((np.sin(xs*xs) + 1.0) * amplitude / 2, (h, 1))
    return a


def make_noise(shape, amplitude):
    return np.random.normal(scale=amplitude, size=shape)


def add_vignetting(a):
    h, w = a.shape
    xs = np.tile(np.linspace(-1.0, 1.0, w), (h, 1))
    ys = np.tile(np.linspace(-1.0, 1.0, h), (w, 1)).transpose()
    v = 1 - ((xs*xs + ys*ys)/2) ** 0.6
    return v*a


def add_bad_pixels(a):
    bad = np.random.choice([1.0, 20.0, 0.05], a.shape, p=[0.99, 0.005, 0.005])
    return a * bad
