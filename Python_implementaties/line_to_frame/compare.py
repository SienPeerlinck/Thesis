"""
Maakt vergelijking tussen files gecomprimmeerd met CCSDS121 of CCSDS122 en hun ongecompimeerde versie
Eerst alle filenames aan elkaar koppelen in array (original, compressed, decompressed, lossless, CR)
test lossless: vergelijking tussen original en decompressed, als lossless 1 toevoegen aan die rij in kolom 4
test compressieverhouding: vergelijking van size van original en compressed, resultaat in kolom 5
Onderverdeling maken van data in subcategorieën (day-side/night-side, dark/light, bitgrootte....) en grafiekjes maken
"""

import numpy as np
from FELICS_vs_IWTSEC.algorithm.felics import Felics
from FELICS_vs_IWTSEC.algorithm.iwtsec import Iwtsec
import subprocess
import matplotlib.pyplot as plt
from collections import deque


def compare(o, c, d, name, fapec=False):
    raw_size_o = o.size * o.itemsize
    if not fapec:
        raw_size_c = len(c)
    else:
        raw_size_c = c.size * c.itemsize
    ratio = raw_size_o / raw_size_c

    if o.all() == d.all():
        lossless = True
    else:
        lossless = False

    data = [name, ratio, lossless]
    return data


def compare_fapec():
    originals = ["frames/1x2a-night-l.raw", "frames/1x2b-night-l.raw", "frames/2x2a-night-l.raw",
                 "frames/2x2b-night-l.raw", "frames/2x2a-day-l.raw", "frames/2x2b-day-l.raw",
                 "frames/3x2a-night-l.raw", "frames/3x2b-night-l.raw", "frames/4x2a-day-l.raw",
                 "frames/4x2b-day-l.raw"]

    data = []

    for i in range(len(originals)):
        print(i)
        subprocess.run(['C:/Program Files (x86)/DAPCOM/WinFAPEC/fapec', '-o', 'c', '-ow', originals[i]])

        subprocess.run(['C:/Program Files (x86)/DAPCOM/WinFAPEC/unfapec', '-o', 'd', '-ow', 'c'])
        original = np.fromfile(originals[i], 'uint32').reshape((288, 384))
        compressed = np.fromfile('c', 'uint32')
        # compressed = deque(c)
        decompressed = np.fromfile('d', 'uint32')
        data_per_file = compare(o=original, c=compressed, d=decompressed, name=originals[i], fapec=True)
        data.append(data_per_file)

    print(data)
    return data


def compare_felics():
    originals = ["frames/1x2a-night-l.raw", "frames/1x2b-night-l.raw", "frames/2x2a-night-l.raw",
                 "frames/2x2b-night-l.raw", "frames/2x2a-day-l.raw", "frames/2x2b-day-l.raw",
                 "frames/3x2a-night-l.raw", "frames/3x2b-night-l.raw", "frames/4x2a-day-l.raw",
                 "frames/4x2b-day-l.raw"]

    data = []
    for i in range(len(originals)):
        im = np.fromfile(originals[i], 'uint32').reshape((288, 384))
        im_base = im.astype(np.uint32)
        bkg = 50000
        im = im_base + bkg
        buf = Felics.encode(im)
        im2 = Felics.decode(buf)
        im2_ = np.array(im2, dtype='uint32')
        data_per_file = compare(o=im, c=buf, d=im2_, name=originals[i])
        data.append(data_per_file)

    print(data)
    return data


def compare_iwtsec():
    originals = ["frames/1x2a-night-l.raw", "frames/1x2b-night-l.raw", "frames/2x2a-night-l.raw",
                 "frames/2x2b-night-l.raw", "frames/2x2a-day-l.raw", "frames/2x2b-day-l.raw",
                 "frames/3x2a-night-l.raw", "frames/3x2b-night-l.raw", "frames/4x2a-day-l.raw",
                 "frames/4x2b-day-l.raw"]

    data = []
    for i in range(len(originals)):
        im = np.fromfile(originals[i], 'uint32').reshape((288, 384))
        buf = Iwtsec.encode(im)
        im2 = Iwtsec.decode(buf)
        buf_ = np.array(buf)
        im2_ = np.array(im2).reshape((288, 384))
        data_per_file = compare(o=im, c=buf, d=im2_, name=originals[i])
        data.append(data_per_file)

    print(data)
    return data


if __name__ == '__main__':
    fapec = compare_fapec()
    felics = compare_felics()
    iwtsec = compare_iwtsec()

    labels, heights1, _ = zip(*fapec)
    _, heights2, _ = zip(*felics)
    _, heights3, _ = zip(*iwtsec)

    # Set the width of the bars
    bar_width = 0.3  # You can adjust this value based on your preference

    # Plotting
    fig, ax = plt.subplots(figsize=(10, 8))

    # Plot bars for each dataset with adjusted x positions
    ax.bar(np.arange(len(labels)) - bar_width, heights1, width=bar_width, color='teal', label='FAPEC')
    ax.bar(np.arange(len(labels)), heights2, width=bar_width, color='coral', label='FELICS')
    ax.bar(np.arange(len(labels)) + bar_width, heights3, width=bar_width, color='purple', label='IWTSEC')

    ax.axhline(y=1, color='gray', linestyle='--')

    # Rotate x-labels for better visibility
    plt.xticks(np.arange(len(labels)), labels, rotation=45, ha='right')

    plt.xlabel('dataset')
    plt.ylabel('compression ratio')
    plt.yscale('linear')
    plt.title('Vergelijking van FAPEC, FELICS en IWTSEC')
    plt.tight_layout()
    plt.legend()

    # Show the plot
    plt.show()
