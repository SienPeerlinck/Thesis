import io

from PIL import Image
from matplotlib import pyplot as plt
import numpy as np

import fapec as fapec
from fapec import Fapec
import detector_sim as sim

DEFAULT_SHAPE = (64, 384)


def benchmark_bkg_level():
    print("Background level benchmark ---------------------------------")
    raw_size = DEFAULT_SHAPE[0] * DEFAULT_SHAPE[1] * 3
    im = sim.make_signal1(DEFAULT_SHAPE, 100000)
    im += sim.make_noise(DEFAULT_SHAPE, 5000)
    im = sim.add_bad_pixels(im)
    im = sim.add_vignetting(im)
    im = np.clip(im, 0, 0xFFFFFF)
    im_base = im.astype(np.uint32)
    for bkg in range(50000, 1000000, 50000):
        im = im_base + bkg
        buf = Fapec.encode(im)
        im2 = Fapec.decode(buf)

        ok = (im == im2).any()
        r = raw_size / len(buf)

            
        print(
            "Background = %7d: %6d -> %6d    %s  (%5.3f)"
            % (bkg, raw_size, len(buf), ok, r)
        )

    print("------------------------------------------------------------")
    print()


def benchmark_amplitude():
    print("Amplitude benchmark ----------------------------------------")
    raw_size = DEFAULT_SHAPE[0] * DEFAULT_SHAPE[1] * 3
    for i in range(14, 24):
        amplitude = (1 << i) - 6000
        im = sim.make_signal1(DEFAULT_SHAPE, amplitude)
        im += sim.make_noise(DEFAULT_SHAPE, 5000)
        im = sim.add_bad_pixels(im)
        im = sim.add_vignetting(im)
        im = np.clip(im, 0, 0xFFFFFF)
        im = im.astype(np.uint32)
        buf = Fapec.encode(im)
        im2 = Fapec.decode(buf)    


        ok = (im == im2).any()
        r = raw_size / len(buf)
        print(
            "Amplitude = %7d: %6d -> %6d    %s  (%5.3f)"
            % (amplitude, raw_size, len(buf), ok, r)
        )
    print("------------------------------------------------------------")
    print()


def benchmark_bits_per_pixel():
    print("Bits per pixel benchmark -----------------------------------")
    raw_size = DEFAULT_SHAPE[0] * DEFAULT_SHAPE[1] * 3
    im = sim.make_signal1(DEFAULT_SHAPE, 2000000)
    im += sim.make_noise(DEFAULT_SHAPE, 5000)
    im = sim.add_bad_pixels(im)
    im = sim.add_vignetting(im)
    im = np.clip(im, 0, 0xFFFFFF)
    im_base = im.astype(np.uint32)
    for i in range(12, 25, 2):
        im = im_base >> (24 - i)
        buf = Fapec.encode(im)
        im2 = Fapec.decode(buf)    

        ok = (im == im2).any()
        r = raw_size / len(buf)

        print("Bits = %2d: %6d -> %6d    %s  (%5.3f)" % (i, raw_size, len(buf), ok, r))

    print("------------------------------------------------------------")
    print()


def benchmark_other_format():
    print("Compare with other format  --------------------------------")
    raw_size = DEFAULT_SHAPE[0] * DEFAULT_SHAPE[1] * 2
    im = sim.make_signal1(DEFAULT_SHAPE, 60000)
    im += sim.make_noise(DEFAULT_SHAPE, 5000)
    im = sim.add_bad_pixels(im)
    im = sim.add_vignetting(im)
    im = np.clip(im, 0, 0xFFFF)
    im = im.astype(np.uint16)
    im_pil = Image.fromarray(im)

    im_buf = io.BytesIO()
    im_pil.save(im_buf, "png")
    print("png: %d -> %d" % (raw_size, len(im_buf.getbuffer())))
    im_buf = io.BytesIO()
    im_pil.save(im_buf, "jpeg2000")
    print("jpeg2000: %d -> %d" % (raw_size, len(im_buf.getbuffer())))
    im_buf = io.BytesIO()
    im_pil.save(im_buf, "tiff", compression="lzma")
    print("tiff lzma: %d -> %d" % (raw_size, len(im_buf.getbuffer())))

    buf = Fapec.encode(im)
    print("fapec: %d -> %d" % (raw_size, len(buf)))

    # plt.imshow(im, cmap="gray", interpolation="nearest")
    # plt.show()
    print("------------------------------------------------------------")
    print()


def benchmark_delta_min():
    print("Delta min benchmark -----------------------------------")
    amplitude = 2000000
    noise = 4096
    bits = amplitude.bit_length()
    raw_size = DEFAULT_SHAPE[0] * DEFAULT_SHAPE[1] * bits / 8
    N = 5
    results = {}
    for j in range(N):
        im = sim.make_signal1(DEFAULT_SHAPE, amplitude)
        im += sim.make_noise(DEFAULT_SHAPE, noise)
        # im = add_bad_pixels(im)
        im = sim.add_vignetting(im)
        im = np.clip(im, 0, 0xFFFFFF)
        im = im / 16
        im = im.astype(np.int32)
        for i in range(8, 20):
            delta_min = 2**i - 1
            buf = Fapec.encode(im)
            im2 = Fapec.decode(buf)
            assert np.array_equal(im, im2)
            bits_per_pixel = len(buf) / im.size * 8
            results[i] = results.setdefault(i, 0) + bits_per_pixel

    xy = list(results.items())
    xy.sort()
    xs = [x for x, _ in xy]
    ys = [y / N for _, y in xy]

    plt.plot(xs, ys)

    # print(delta_min, im.size, len(buf), len(buf)/im.size*8)
    # plt.imshow(im2, cmap="gray", interpolation='nearest')
    plt.show()
    print("------------------------------------------------------------")
    print()


def check_delta_compression():
    common_noise = 10000
    c_noise = sim.make_noise(DEFAULT_SHAPE, common_noise)
    c_delta_min = 8191

    amplitude1 = 1000000
    amplitude2 = 1001000
    noise = 1000
    delta_min = 1023

    im1 = sim.make_signal1(DEFAULT_SHAPE, amplitude1)
    im1 += c_noise + sim.make_noise(DEFAULT_SHAPE, noise)
    im1 = sim.add_vignetting(im1)
    im1 = np.clip(im1, 0, 0xFFFFFF)
    im1 = im1.astype(np.int32)
    im_pil = Image.fromarray(im1)
    im_pil.save("image.tiff")
    imp_pil2 = Image.open("image.tiff")
    ok = (im1 == np.asarray(imp_pil2)).all()
    print(ok, im_pil.mode)

    im2 = sim.make_signal1(DEFAULT_SHAPE, amplitude1)
    im2 += c_noise + sim.make_noise(DEFAULT_SHAPE, noise)
    im2 = sim.add_vignetting(im2)
    im2 = np.clip(im2, 0, 0xFFFFFF)
    im2 = im2.astype(np.int32)

    im = im2 - im1
    im -= np.min(im)

    buf = Fapec.encode(im1, delta_min=c_delta_min)
    im3 = Fapec.decode(buf)
    assert np.array_equal(im1, im3)
    print(c_delta_min, im1.size, len(buf), len(buf) / im1.size * 8)

    buf = Fapec.encode(im, delta_min=delta_min)
    im3 = Fapec.decode(buf)
    assert np.array_equal(im, im3)
    print(delta_min, im.size, len(buf), len(buf) / im.size * 8)


def run(args):
    benchmark_bkg_level()  
    benchmark_amplitude()
    benchmark_bits_per_pixel()
    benchmark_other_format()
    # benchmark_delta_min()
    # check_delta_compression()
