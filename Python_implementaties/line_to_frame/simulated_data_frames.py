import sys

import numpy as np
import matplotlib.pyplot as plt


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


if __name__ == "__main__":
    s = SimulatedDataFrames(sys.argv[1])

    # s.init_frame(288)
    # s.add_noise(10)
    # s.add_vignetting(0.5)
    # a = s.get_frame()

    s.init_frame(288)
    s.add_noise()
    s.add_vignetting()
    a = s.get_frame()

    plt.imshow(a, cmap="gray")
    plt.show()
