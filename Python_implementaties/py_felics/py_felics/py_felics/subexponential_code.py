import numpy as np
from context import Context

class SubexponentialCode(object):
    class __SE_Context(object):
        K_NBITS     = 24
        DELTA_NBITS = 24

        def __init__(self):
            self.__k_table = np.zeros((self.K_NBITS, self.DELTA_NBITS),
                                      np.int32)
            self.__best_ks = np.zeros((self.DELTA_NBITS,), np.int32)
                

        def estimate_k(self, delta):
            #return 8
            delta_bits = delta.bit_length()
            k = self.__best_ks[delta_bits]
            return int(k)

        def update(self, x, delta, k):
            delta_bits = delta.bit_length()
            best_k = x.bit_length()     # FixMe: more tests needed...
            cnt = self.__k_table[best_k, delta_bits] + 1
            self.__k_table[best_k, delta_bits] = cnt
            if cnt > self.__k_table[k, delta_bits]:
                self.__best_ks[delta_bits] = best_k

        def print_info(self):
            print(self.__best_ks)


    def __init__(self, bs, *args):
        self.__bs = bs
        if len(args) >= 1:
            if type(args[0]) == Context:
                self.__context = args[0]
            else:
                self.__context = self.__SE_Context()
        else:
            self.__context = self.__SE_Context()
    
    # def count_bits(self, x, k):
    #     self.__context.__cntrs
    #     return cnt

    def push(self, x, delta):
        k = self.__context.estimate_k(delta)
        print(x)
        self.__context.update(x, delta, k)
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

    def pop(self, delta):
        k = self.__context.estimate_k(delta)
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
        self.__context.update(x, delta, k)
        return x
    
    def print_ctx_info(self):
        self.__context.print_info()

