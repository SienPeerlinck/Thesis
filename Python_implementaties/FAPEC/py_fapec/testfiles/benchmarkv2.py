import io

from PIL import Image
from matplotlib import pyplot as plt
import numpy as np
from fapyc import Fapyc, Unfapyc


import fapec as felics
from fapec import Fapec
import detector_sim as sim
from context import Context

DEFAULT_SHAPE = (64, 384)
FAPEC_HOME = "C:/Users/sienp/Documents/school/IIW4/Thesis/Python_implementaties/py_fapec/FAPEC/py_fapec/fapeclic.dat"


def bench_bkg_level():
    print("Background level benchmark ---------------------------------")
    raw_size = DEFAULT_SHAPE[0] * DEFAULT_SHAPE[1] * 2.5
    im = sim.make_signal1(DEFAULT_SHAPE, 50000)
    im += sim.make_noise(DEFAULT_SHAPE, 5000)
    im = sim.add_bad_pixels(im)
    im = sim.add_vignetting(im)
    im = np.clip(im, 0, 0xfffff)
    im_base = im.astype(np.uint32)
    print("Background = bkg: raw_size -> encoded size    lossless?")
    for bkg in range(50000, 1000000, 50000):  
        im = im_base + bkg
        file = im.tobytes()
        ctx = Context(0, 20, 13)
        buf = Fapyc(buffer = file)
        buf.compress_tabtxt(sep1=',')
        ctx = Context(0, 20, 13)
        im2 = Unfapyc(buffer=buf.outputBuffer)
        im2.decompress()
        print("Ratio =", round(float(len(im))/len(buf.outputBuffer), 4))

        ok = (im == im2).any()
        print("Background = %7d: %6d -> %6d    %s" %
              (bkg, raw_size, len(buf.outputBuffer), ok))
        # plt.imshow(im2, cmap="gray", interpolation='nearest')
        # plt.title("decoded image with background %7d" %bkg)
        # plt.show()
    print("------------------------------------------------------------")
    print()


def bench_amplitude():
    print("Amplitude benchmark ----------------------------------------")
    raw_size = DEFAULT_SHAPE[0] * DEFAULT_SHAPE[1] * 2.5
    print("Amplitude = iteratie: raw_size -> encoded size    lossless?")
    for i in range(14, 21):
        amplitude = (1 << i) - 6000
        im = sim.make_signal1(DEFAULT_SHAPE, amplitude)
        im += sim.make_noise(DEFAULT_SHAPE, 5000)
        im = sim.add_bad_pixels(im)
        im = sim.add_vignetting(im)
        im = np.clip(im, 0, 0xfffff)
        im = im.astype(np.uint32)

        ctx = Context(0, 20, 13)
        buf = Fapec.encode(im)
        ctx = Context(0, 20, 13)
        im2 = Fapec.decode(buf)

        ok = (im == im2).any()
        print("Amplitude = %7d: %6d -> %6d    %s" %
              (amplitude, raw_size, len(buf), ok))
    print("------------------------------------------------------------")
    print()


def bench_bits_per_pixel():
    print("Bits per pixel benchmark -----------------------------------")
    raw_size = DEFAULT_SHAPE[0] * DEFAULT_SHAPE[1] * 2.5
    im = sim.make_signal1(DEFAULT_SHAPE, 1000000)
    im += sim.make_noise(DEFAULT_SHAPE, 5000)
    im = sim.add_bad_pixels(im)
    im = sim.add_vignetting(im)
    im = np.clip(im, 0, 0xfffff)
    im_base = im.astype(np.uint32)
    print("Bits = iteratie: raw_size -> encoded size    lossless?")
    for i in range(12, 21, 2):
        im = im_base >> (20 - i)

        ctx = Context(0, 20, 13)
        buf = Fapec.encode(im)
        ctx = Context(0, 20, 13)
        im2 = Fapec.decode(buf)

        ok = (im == im2).any()
        print("Bits = %2d: %6d -> %6d    %s" %
              (i, raw_size, len(buf), ok))

    print("------------------------------------------------------------")
    print()


def bench_context():
    print("Context (subexponential k) benchmark -----------------------------------")
    raw_size = DEFAULT_SHAPE[0] * DEFAULT_SHAPE[1] * 2.5
    im = sim.make_signal1(DEFAULT_SHAPE, 1000000)
    im += sim.make_noise(DEFAULT_SHAPE, 5000)
    im = sim.add_bad_pixels(im)
    im = sim.add_vignetting(im)
    im = np.clip(im, 0, 0xfffff)
    im = im.astype(np.uint32)
    print("Context (m  e  k): raw_size -> encoded size    lossless?")
    for m in range(4):
        for k in range(10, 15):
            e = 20 - m

            ctx = Context(m, e, k)
            buf = Fapec.encode(im)
            ctx = Context(m, e, k)
            im2 = Fapec.decode(buf)

            ok = (im == im2).any()
            print("Context (m=%2d e=%2d k=%d) : %6d -> %6d    %s" %
                  (m, e, k, raw_size, len(buf), ok))

    print("------------------------------------------------------------")
    print()


def bench_other_format():
    print("Compare with other format  --------------------------------")
    raw_size = DEFAULT_SHAPE[0] * DEFAULT_SHAPE[1] * 2
    im = sim.make_signal1(DEFAULT_SHAPE, 60000)
    im += sim.make_noise(DEFAULT_SHAPE, 5000)
    im = sim.add_bad_pixels(im)
    im = sim.add_vignetting(im)
    im = np.clip(im, 0, 0xffff)
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

    ctx = Context(0, 16, 12)
    buf = Fapec.encode(im)
    print("felics: %d -> %d" % (raw_size, len(buf)))

    # ctx.plot_curve()
    # plt.imshow(im, cmap="gray", interpolation='nearest')
    # plt.title("original image")
    # plt.show()
    print("------------------------------------------------------------")
    print()

def bench_context2():
    print("Context (subexponential k) benchmark -----------------------------------")
    raw_size = DEFAULT_SHAPE[0] * DEFAULT_SHAPE[1] * 2.5
    im = sim.make_signal1(DEFAULT_SHAPE, 1000000)
    im += sim.make_noise(DEFAULT_SHAPE, 10000)
    im = sim.add_bad_pixels(im)
    im = sim.add_vignetting(im)
    im = np.clip(im, 0, 0xfffff)
    im = im.astype(np.uint32)

    ctx = Context(0, 20, 16)
    buf = Fapec.encode(im)
    print("felics: %d -> %d" % (raw_size, len(buf)))

    # ctx.plot_curve()

def simple_check():
    print("simple check benchmark -----------------------------------")
    amplitude = 8000000
    noise = 4096
    bits = amplitude.bit_length()
    raw_size = DEFAULT_SHAPE[0] * DEFAULT_SHAPE[1] * bits / 8
    N = 5
    results = {}
    for j in range(N):
        print(j)
        im = sim.make_signal1(DEFAULT_SHAPE, amplitude)
        im += sim.make_noise(DEFAULT_SHAPE, noise)
        # im = add_bad_pixels(im)
        im = sim.add_vignetting(im)
        im = np.clip(im, 0, 0xffffff)
        im = im / 16  # ??
        im = im.astype(np.int32)
        for i in range(8,20):
            delta_min = 2**i - 1
            buf = Fapec.encode(im)
            im2 = Fapec.decode(buf)
            assert np.array_equal(im, im2)
            bits_per_pixel = len(buf)/im.size*8
            results[i] = results.setdefault(i, 0) + bits_per_pixel

    
    xy = list(results.items())
    xy.sort()
    xs = [x for x, _ in xy]
    ys = [y / N for _, y in xy]

    plt.plot(xs, ys)
    plt.title("simple check")
    plt.xlabel("i")
    plt.show()

    print("delta_min = ", delta_min, "original size = ", im.size, "compressed size = ", len(buf), "bits per pixel = ", len(buf)/im.size*8)
    plt.imshow(im2, cmap="gray", interpolation='nearest')
    plt.title("compressed image")
    plt.show()

def check_delta_compression():
    print("check delta compression benchmark -----------------------------------")
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
    im1 = np.clip(im1, 0, 0xffffff)
    im1 = im1.astype(np.int32)
    im_pil = Image.fromarray(im1)
    im_pil.save("image.tiff")
    imp_pil2 = Image.open("image.tiff")
    ok = (im1 == np.asarray(imp_pil2)).all()
    print(ok, im_pil.mode)
    
    im2 = sim.make_signal1(DEFAULT_SHAPE, amplitude1)
    im2 += c_noise + sim.make_noise(DEFAULT_SHAPE, noise)
    im2 = sim.add_vignetting(im2)
    im2 = np.clip(im2, 0, 0xffffff)
    im2 = im2.astype(np.int32)

    im = im2 - im1
    im -= np.min(im)

    buf = Fapec.encode(im1)
    im3 = Fapec.decode(buf)
    assert np.array_equal(im1, im3)
    print(c_delta_min, im1.size, len(buf), len(buf)/im1.size*8)

    buf = Fapec.encode(im)
    im3 = Fapec.decode(buf)
    assert np.array_equal(im, im3)
    print(delta_min, im.size, len(buf), len(buf)/im.size*8)

def run(args):
    print("running benchmark")
    bench_bkg_level()   # No effect on compressed ratio
    bench_amplitude()   # Higher amplitude --> smaller compression ratio
    bench_bits_per_pixel()  # More bits per pixel --> smaller compression ratio
    #bench_context()     # Cannot be tested
    bench_other_format()  # Felics has a higher compression ratio compared o png, jpeg2000 and tiff lzma
    #bench_context2()
    simple_check()
    #check_delta_compression()
    print("benchmark finished")
