class AdjustedBinCode(object):
    def __init__(self, bs):
        self.__bs = bs

    def push(self, x, delta):
        n = delta.bit_length()
        a = (1 << n) - delta - 1    # n - 1 bits
        b = (1 << n) - (a << 1)     # n bits

        half_b = b >> 1
        if x < half_b:
            self.__bs.push_bits((a + x) << 1, n)
        else:
            x -= half_b
            if x < a:
                self.__bs.push_bits(x, n - 1)
            else:
                self.__bs.push_bits((x << 1) | 1, n)

    def pop(self, delta):
        n = delta.bit_length()
        a = (1 << n) - delta - 1     # n - 1 bits
        b = (1 << n) - (a << 1)      # n bits

        half_b = b >> 1
        y = self.__bs.pop_bits(n-1)
        if y < a:
            return y + half_b
        elif self.__bs.pop_bits(1) == 0:
            return y - a
        else:
            return y + half_b

