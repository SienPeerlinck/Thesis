import numpy as np
from collections import deque


class SubexponentialCode(object):
    class __SE_Context(object):
        K_NBITS = 24
        DELTA_NBITS = 24

        def __init__(self):
            self.__k_table = np.zeros((self.K_NBITS, self.DELTA_NBITS), np.int32)
            self.__best_ks = np.zeros((self.DELTA_NBITS,), np.int32)
            
        def estimate_k(self, d):
            # return 12
            k = d-2
            if k<=0:
                return 1
            else:
                return k

        def update(self, x, delta, k):
            delta_bits = delta.bit_length()
            best_k = x.bit_length() - 2 # FixMe: more tests needed...
            cnt = self.__k_table[best_k, delta_bits] + 1
            self.__k_table[best_k, delta_bits] = cnt
            if cnt > self.__k_table[k, delta_bits]:
                self.__best_ks[delta_bits] = best_k

        def print_info(self):
            print(self.__best_ks)

    __k_list = deque([])
    def __init__(self, bs):
        self.__bs = bs
        self.__context = self.__SE_Context()

    def push(self, x):
        d = x.bit_length()
        k = self.__context.estimate_k(d)
        SubexponentialCode.__k_list.append(k)
        # self.__context.update(x, delta, k)
        b = x.bit_length() - 1
        if b < k:
            b = k
            u = 0
        else:
            u = b - k + 1

        # Unary code of u
        for i in range(u):
            self.__bs.push_bits(1, 1)
        self.__bs.push_bits(0, 1)

        # b least significant bit of x
        mask = (1 << b) - 1
        self.__bs.push_bits(x & mask, b)
        
    def pop(self):
        # k = self.__context.estimate_k()
        # print("pop: ", len(SubexponentialCode.__k_list))
        k = SubexponentialCode.__k_list.popleft()
        # except IndexError: k=12
        u = 0
        while self.__bs.pop_bits(1) == 1:
            u += 1
        if u == 0:
            b = k
            x = 0
        else:
            b = u + k - 1
            x = 1
        x <<= b
        x |= self.__bs.pop_bits(b)
        # self.__context.update(x, delta, k)
        return x

    def print_ctx_info(self):
        self.__context.print_info()
