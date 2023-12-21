"""
Reads DAT file whit one row of simulated data (and header)
Returns data frame with added noise, vignetting and bad pixels
"""
import io

import numpy as np
import pandas as pd
from PIL import Image
from matplotlib import pyplot as plt
import FELICS_vs_IWTSEC.algorithm.detector_sim as sim

HEIGHT = 288
WIDTH = 384
DEFAULT_SHAPE = (HEIGHT, WIDTH)


class SimulatedDataFrames(object):
    def __init__(self, filename):
        with open(filename) as fd:
            for _ in range(4):
                fd.readline()
            self.__pix_arr = []
            self.__dark_arr = []
            self.__light_arr = []
            for line in fd:
                s = line.split(",")
                self.__pix_arr.append(int(s[0]))
                self.__dark_arr.append(float(s[1]))
                self.__light_arr.append(float(s[2]))

    def init_frame(self, height=288):
        self.__cur_frame = np.tile(self.__light_arr, (height, 1))

    def add_noise(self, level=None):
        # By default, the noise level is the square root of the dark level
        if level is None:
            level = self.__dark_arr[0] ** 0.5
        noise = np.random.normal(scale=level, size=self.__cur_frame.shape)
        self.__cur_frame += noise

    def add_vignetting(self, gamma=1.0):
        offset = self.__dark_arr[0]
        height, width = self.__cur_frame.shape
        xs = np.tile(np.linspace(-1.0, 1.0, width), (height, 1))
        ys = np.tile(np.linspace(-1.0, 1.0, height), (width, 1)).transpose()
        # v = 1 - ((xs * xs + ys * ys) / 2) ** gamma
        v = (np.cos(xs * np.pi / 2) * np.cos(ys * np.pi / 2)) ** gamma
        self.__cur_frame -= offset
        self.__cur_frame *= v
        self.__cur_frame += offset

    def get_frame(self):
        return self.__cur_frame.round().astype(int)


if __name__ == '__main__':
    lijnen = ["data/1x2a-night.dat", "data/1x2b-night.dat", "data/2x2a-night.dat", "data/2x2b-night.dat",
              "data/2x2a-day.dat", "data/2x2b-day.dat", "data/3x2a-night.dat", "data/3x2b-night.dat",
              "data/4x2a-day.dat", "data/4x2b-day.dat"]
    frames = ["frames/1x2a-night-l.raw", "frames/1x2b-night-l.raw", "frames/2x2a-night-l.raw",
              "frames/2x2b-night-l.raw", "frames/2x2a-day-l.raw", "frames/2x2b-day-l.raw",
              "frames/3x2a-night-l.raw", "frames/3x2b-night-l.raw", "frames/4x2a-day-l.raw",
              "frames/4x2b-day-l.raw"]
    for i in range(len(lijnen)):
        s = SimulatedDataFrames(lijnen[i])
        s.init_frame(HEIGHT)
        s.add_noise()
        s.add_vignetting()
        a = s.get_frame()
        a.tofile(frames[i])
        fig, ax = plt.subplots(figsize=(10, 8))
        plt.imshow(a, cmap="gray")
        plt.title(lijnen[i])
        plt.show()
