from collections import deque

class BitStream(object):
    def __init__(self, byte_arr=[]):
        self.__bits = 0
        self.__bytes = deque(byte_arr)
        self.__bit_cntr = 0

    def push_bits(self, v, n):
        self.__bits <<= n
        mask = (1 << n) - 1
        self.__bits |= v & mask
        self.__bit_cntr += n

        while self.__bit_cntr >= 8:
            self.__bit_cntr -= 8
            mask = 0xff << self.__bit_cntr
            byte = (self.__bits & mask) >> self.__bit_cntr
            self.__bits &= ~mask
            self.__bytes.append(byte)

    def pop_bits(self, n):
        while n > self.__bit_cntr:
            try: self.__bits = (self.__bits << 8) | self.__bytes.popleft()
            except:
                return
            self.__bit_cntr += 8

        self.__bit_cntr -= n
        mask = ((1 << n) - 1) << self.__bit_cntr
        # print(self.__bit_cntr, self.__bits, mask)
        v = (self.__bits & mask) >> self.__bit_cntr
        self.__bits &= ~mask
        return int(v)

    def flush(self):
        assert self.__bit_cntr < 8
        if self.__bit_cntr > 0:
            byte = self.__bits << (8 - self.__bit_cntr)
            self.__bytes.append(byte)
            self.__bit_cntr = 0
            self.__bits = 0

    def get_bytes(self):
        if self.__bit_cntr == 0:
            return self.__bytes
        else:
            return None

