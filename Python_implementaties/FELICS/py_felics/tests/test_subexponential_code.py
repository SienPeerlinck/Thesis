from py_felics.subexponential_code import push_bits, pop_bits
from py_felics.bit_stream import BitStream


def test_ex1():
    bs = BitStream()
    push_bits(bs, 9, 0)
    bs.flush()
    print(["%02x" % b for b in bs.get_bytes()])
    x = pop_bits(bs, 0)
    assert x == 9



def test_ex2():
    for k in range(16):
        bs = BitStream()
        for x in range(250):
            push_bits(bs, x, k)
        bs.flush()

        for x in range(250):
            assert x == pop_bits(bs, k)

        bs = BitStream()
        for x in range(800000, 800500):
            push_bits(bs, x, k)
        bs.flush()

        for x in range(800000, 800500):
            assert x == pop_bits(bs, k)


def test_ex3():
    bs = BitStream()
    for x in range(100000):
        k = ((x + 11) * 7) % 17
        push_bits(bs, x, k)

    bs.flush()
    for x in range(100000):
        k = ((x + 11) * 7) % 17
        assert x == pop_bits(bs, k)

