import numpy as np

from algorithm.bit_stream import BitStream
from algorithm.adjusted_bin_code import AdjustedBinCode
from algorithm.subexponential_code import SubexponentialCode
from algorithm.golomb_rice_code import RiceCode
import algorithm.LeGall53dwt as lg


class Felics(object):
    VERSION = 0x01
    DEFAULT_DELTA_MIN = 255

    # Header Structure
    FELICS_VERSION_BITS = 8
    DELTA_MIN_BITS = 24
    IMG_WIDTH_BITS = 12
    IMG_HEIGHT_BITS = 12
    BITS_PER_PIXEL_BITS = 5

    def __init__(self, delta_min=DEFAULT_DELTA_MIN, use_subexp_code=True):
        self.__delta_min = delta_min
        self.__use_subexp_code = use_subexp_code

    @classmethod
    def encode(cls, im, *args, **kw):
        return cls(*args, **kw).__encode(im)

    @classmethod
    def decode(cls, byte_arr, *args, **kw):
        return cls(*args, **kw).__decode(byte_arr)

    def __encode(self, im):
        bs = BitStream()
        ab_code = AdjustedBinCode(bs)
        if self.__use_subexp_code:
            out_of_range_code = SubexponentialCode(bs)
        else:
            out_of_range_code = RiceCode(bs)

        # FELICS Header
        bs.push_bits(self.VERSION, self.FELICS_VERSION_BITS)
        bs.push_bits(self.__delta_min, self.DELTA_MIN_BITS)

        height, width = im.shape
        bs.push_bits(width, self.IMG_WIDTH_BITS)
        bs.push_bits(height, self.IMG_HEIGHT_BITS)

        p0_len = int(im[0, 0]).bit_length()
        p1_len = int(im[0, 1]).bit_length()
        bits_per_pixel = max(p0_len, p1_len)
        bs.push_bits(bits_per_pixel, self.BITS_PER_PIXEL_BITS)

        dt = np.dtype('i8')
        coeff = lg.dwt(im).astype(dt)

        for y in range(height):
            for x in range(width):
                if y == 0:
                    if x < 2:
                        bs.push_bits(coeff[y, x], bits_per_pixel)
                        continue
                    else:
                        l = coeff[y, x - 2]
                        h = coeff[y, x - 1]
                else:
                    if x == 0:
                        l = coeff[y - 1, 0]
                        h = coeff[y - 1, 1]
                    else:
                        l = coeff[y - 1, x]
                        h = coeff[y, x - 1]
                l = int(l)
                h = int(h)
                if l > h:
                    l, h = h, l
                delta = h - l

                if delta < self.__delta_min:
                    if l < self.__delta_min // 2:
                        l = 0
                    else:
                        l -= self.__delta_min // 2
                    h = l + self.__delta_min
                    delta = self.__delta_min

                p = int(coeff[y, x])
                if l <= p <= h:
                    # In range
                    bs.push_bits(0, 1)
                    if delta != 0:
                        ab_code.push(p - l, delta)
                else:
                    # Out of range
                    bs.push_bits(1, 1)

                    if p < l:
                        # Below range
                        bs.push_bits(0, 1)
                        e = l - p - 1
                    else:
                        assert p > h
                        # Above range
                        bs.push_bits(1, 1)
                        e = p - h - 1
                    out_of_range_code.push(e)
        bs.flush()
        # ctx.print_cntrs()
        # ctx.plot_curve()
        return bs.get_bytes()

    def __decode(self, byte_arr):
        bs = BitStream(byte_arr)
        ab_code = AdjustedBinCode(bs)
        if self.__use_subexp_code:
            out_of_range_code = SubexponentialCode(bs)
        else:
            out_of_range_code = RiceCode(bs)

        version = bs.pop_bits(self.FELICS_VERSION_BITS)
        delta_min = bs.pop_bits(self.DELTA_MIN_BITS)

        width = bs.pop_bits(self.IMG_WIDTH_BITS)
        height = bs.pop_bits(self.IMG_HEIGHT_BITS)
        im = np.empty((height, width), np.uint64).astype('i8')

        bits_per_pixel = bs.pop_bits(self.BITS_PER_PIXEL_BITS)

        for y in range(height):
            for x in range(width):
                if y == 0:
                    if x < 2:
                        im[y, x] = bs.pop_bits(bits_per_pixel)
                        continue
                    else:
                        l = im[y, x - 2]
                        h = im[y, x - 1]
                else:
                    if x == 0:
                        l = im[y - 1, 0]
                        h = im[y - 1, 1]
                    else:
                        l = im[y - 1, x]
                        h = im[y, x - 1]
                l = int(l)
                h = int(h)
                if l > h:
                    l, h = h, l
                delta = h - l

                if delta < delta_min:
                    if l < delta_min // 2:
                        l = 0
                    else:
                        l -= delta_min // 2
                    h = l + delta_min
                    delta = delta_min

                if bs.pop_bits(1) == 0:
                    # In range
                    if delta != 0:
                        e = ab_code.pop(delta)
                    else:
                        e = 0
                    im[y, x] = l + e
                else:
                    # Out of range
                    above_range = bs.pop_bits(1)
                    e = out_of_range_code.pop()
                    if above_range == 0:
                        p = l - e - 1
                    else:
                        p = e + h + 1
                    # print("(x=%3d p=%d)" % (x,p))
                    im[y, x] = p
        # se_code.print_ctx_info()
        coeff = lg.idwt(im)
        return coeff
