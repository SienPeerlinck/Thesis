import numpy as np
from matplotlib import pyplot as plt
import algorithm.subexponential_code as subexponential_code


class Context(object):
    def __init__(self, m_bits, e_max, k_max):
        self.__cntrs = np.zeros((1 << m_bits, e_max+1, k_max+1))
        self.__m_bits = m_bits
        self.__e_max = e_max
        self.__k_max = k_max

    def update(self, d, x):
        n = d.bit_length()
        e = max(0, n - self.__m_bits)
        assert e <= self.__e_max
        m = d >> e
        for k in range(self.__k_max+1):
            cnt = subexponential_code.count_bits(int(x), k)
            self.__cntrs[m, e, k] += cnt

    def choose_k(self, d):
        n = d.bit_length()
        e = max(0, n - self.__m_bits)
        assert e <= self.__e_max
        m = d >> e
        k = int(np.argmin(self.__cntrs[m, e]))
        if self.__cntrs[m, e, k] > 0:
            return k
        else:
            if n > 8:
                return n - 4
            return 4

    def plot_curve(self):
        xs = []
        ys = []
        for e in range(self.__e_max):
            for m in range(1 << self.__m_bits):
                k = int(np.argmin(self.__cntrs[m, e]))
                cnt = self.__cntrs[m, e, k]
                if cnt > 0:
                    xs.append(e)
                    ys.append(k)
        plt.plot(xs, ys)
        plt.show()

    def print_cntrs(self):
        print(self.__cntrs)

class Context0(object):
    def __init__(self, k_table):
        self.__k_table = k_table

    def update(self, d, x):
        pass
    
    def choose_k(self, d):
        n = d.bit_length()
        assert n <= 20
        #print(n)
        return self.__k_table[n]

