class RiceCode(object):
    def __init__(self, bs):
        self.__bs = bs

    def __estimate_k(self, delta):
        delta_bits = delta.bit_length()
        k = max(delta_bits - 8, 12)
        return k

    def push(self, x, delta):
        k = self.__estimate_k(delta)
        mask = (1 << k) - 1
        l = x & mask
        h = x >> k
        for i in range(h):
            self.__bs.push_bits(1, 1)
        self.__bs.push_bits(0, 1)
        self.__bs.push_bits(l, k)

    def pop(self, delta):
        k = self.__estimate_k(delta)
        h = 0
        while self.__bs.pop_bits(1) == 1:
            h += 1
        l = self.__bs.pop_bits(k)
        return (h << k) | l
