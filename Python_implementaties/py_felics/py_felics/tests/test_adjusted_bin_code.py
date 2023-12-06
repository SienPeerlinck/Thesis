from py_felics.adjusted_bin_code import encode, decode, push_bits, pop_bits
from py_felics.bit_stream import BitStream

def test_ex1():
    r = 10

    y, n = encode(0, r)
    assert (y, n) == (10, 4)

    y, n = encode(1, r)
    assert (y, n) == (12, 4)

    y, n = encode(3, r)
    assert (y, n) == (0, 3)


def test_ex2():
    for r in range(0, 70):
        for x in range(r+1):
            bits, n = encode(x, r)
            y = decode(bits, r)
            assert x == y


def test_ex3():
    assert encode(0, 1) == (0, 1)
    assert encode(1, 1) == (1, 1)
    assert encode(0, 0) == (1, 0)
    assert encode(0xfffff, 0xfffff) == (0xfffff, 20)
    bits, n = encode(0xfff00, 0xffff0)
    assert decode(bits, 0xffff0) == 0xfff00

def test_ex4():
    bs = BitStream()
    for x in range(100000):
        r = x + ((x + 11) * 7) % 17
        push_bits(bs, x, r)

    bs.flush()
    for x in range(100000):
        r = x + ((x + 11) * 7) % 17
        assert x == pop_bits(bs, r)

