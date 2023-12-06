import numpy as np

from py_felics.detector_sim import *
from py_felics.felics import encode, decode
from py_felics.bit_stream import BitStream
from py_felics.context import Context

DEFAULT_SHAPE = (288, 384)
#DEFAULT_SHAPE = (288, 44)


def test_ex1():
    im = make_signal1(DEFAULT_SHAPE, 1000000)
    im += make_noise(DEFAULT_SHAPE, 5000)
    im = add_bad_pixels(im)
    im = add_vignetting(im)
    im = np.clip(im, 0, 0xfffff)
    im = im.astype(np.uint32)

    ctx = Context(6, 14, 16)
    buf = encode(im, 20, ctx)
    print(len(buf))

    ctx = Context(6, 14, 16)
    im2 = decode(buf, 20, ctx)

    assert (im == im2).any()
