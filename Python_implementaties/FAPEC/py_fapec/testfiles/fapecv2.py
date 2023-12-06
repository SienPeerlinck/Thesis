import pywt as wt # PyWavelets library
import numpy as np
from golomb_rice_code import GolombRiceCode
from bit_stream import BitStream
from subexponential_code import SubexponentialCode
import LeGall53dwt as lg
from adjusted_bin_code import AdjustedBinCode
from huffman_code import HuffmanCode
import burrow_wheeler_transform as bw


class Fapec(object):
    VERSION = 0x01
    
    # Header Structure
    FAPEC_VERSION_BITS = 8
    IMG_WIDTH_BITS = 12
    IMG_HEIGHT_BITS = 12
    BITS_PER_PIXEL_BITS = 5

    def __init__(self):  
       return
    
    @classmethod
    def encode(cls, im, *args, **kw):
        return cls(*args, **kw).__encode(im)

    @classmethod
    def decode(cls, byte_arr, *args, **kw):
        return cls(*args, **kw).__decode(byte_arr)
    
    def __encode(self, im):
        bs = BitStream()
        gr_code = GolombRiceCode()
        se_code = SubexponentialCode(bs)
        ab_code = AdjustedBinCode(bs)
        hf_code = HuffmanCode(bs)

        # FAPEC Header
        bs.push_bits(self.VERSION, self.FAPEC_VERSION_BITS)

        height, width = im.shape
        bs.push_bits(width, self.IMG_WIDTH_BITS)
        bs.push_bits(height, self.IMG_HEIGHT_BITS)

        p0_len = int(im[0, 0]).bit_length()
        p1_len = int(im[0, 1]).bit_length()
        bits_per_pixel = max(p0_len, p1_len)
        bs.push_bits(bits_per_pixel, self.BITS_PER_PIXEL_BITS)

        # pre-processing
        print(1 in im)

        # Entropy coding
        for i in range(im.shape[0]):
            coeff = bw.bwt(str(im[i]))
            n = coeff[i]
            se_code.push(n, 15)

        bs.flush()

        return bs.get_bytes()
    
    def __decode(self, byte_arr):
        bs = BitStream(byte_arr)
        gr_code = GolombRiceCode()
        se_code = SubexponentialCode(bs)
        ab_code = AdjustedBinCode(bs)
        hf_code = HuffmanCode(bs)

        m = 7

        version = bs.pop_bits(self.FAPEC_VERSION_BITS)

        width = bs.pop_bits(self.IMG_WIDTH_BITS) 
        height = bs.pop_bits(self.IMG_HEIGHT_BITS)
        coeff = np.empty((height, width), np.uint32)

        bits_per_pixel = bs.pop_bits(self.BITS_PER_PIXEL_BITS)

        # entropy decoding
        for y in range(height):
            for x in range(width):
                p = se_code.pop(15)
                coeff[y, x] = p

        # post-processing
        im = lg.idwt(coeff)
        return im
