# https://tim.cogan.dev/wavelet/
import numpy as np
from numpy import ndarray

class WaveletImage:
    def __init__(self, image: ndarray, axis: int = 1, levels: int = 2) -> None:
        self.axis = axis
        self.lo, self.hi = self.transform(image, self.axis, levels)

    @property
    def pixels(self) -> ndarray:
        lo = norm_image(self.lo if isinstance(self.lo, ndarray) else self.lo.pixels)
        hi = norm_image(self.hi if isinstance(self.hi, ndarray) else self.hi.pixels)
        return np.concatenate([lo, hi], axis=self.axis)

    def inverse_transform(self) -> ndarray:
        lo: ndarray = self.lo if isinstance(self.lo, ndarray) else self.lo.inverse_transform()
        hi: ndarray = self.hi if isinstance(self.hi, ndarray) else self.hi.inverse_transform()
        evens = (lo + hi) // 2
        odds = (lo - hi) // 2
        return interleave(evens, odds, axis=self.axis)

    @staticmethod
    def transform(image: ndarray, axis: int, levels: int) -> tuple[ndarray, ndarray]:
        if axis == 0:
            evens, odds = image[::2, :], image[1::2, :]
        elif axis == 1:
            evens, odds = image[:, ::2], image[:, 1::2]
        else:
            raise ValueError(f"axis '{axis}' must be 0 or 1")

        lo = WaveletImage(evens + odds, abs(axis - 1), levels - axis) if levels else evens + odds
        hi = WaveletImage(evens - odds, axis=0, levels=0) if axis == 1 else evens - odds

        return lo, hi


def norm_image(x: ndarray) -> ndarray:
    return (x - x.min()) / (x.max() - x.min())


def interleave(a: ndarray, b: ndarray, axis: int) -> ndarray:
    rows, cols = a.shape
    rows, cols = (rows * 2, cols) if axis == 0 else (rows, cols * 2)
    out = np.empty((rows, cols), dtype=a.dtype)
    if axis == 0:
        out[0::2] = a
        out[1::2] = b
    elif axis == 1:
        out[:, 0::2] = a
        out[:, 1::2] = b
    else:
        raise ValueError("interleave only supports axis of 0 or 1")
    return out