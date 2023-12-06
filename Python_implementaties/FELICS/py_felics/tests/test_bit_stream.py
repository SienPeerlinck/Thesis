from py_felics.bit_stream import BitStream

def test_ex1():
    bs = BitStream()
    for i in range(8):
        bs.push_bits(0,1)
        bs.push_bits(1,1)
    assert list(bs.get_bytes()) == [0x55, 0x55]

    bs.push_bits(0x07, 6)
    assert bs.get_bytes() is None

    bs.flush()
    assert list(bs.get_bytes()) == [0x55, 0x55, 0x1c]

    bs.push_bits(0x12345678, 32)
    assert list(bs.get_bytes()) == [0x55, 0x55, 0x1c, 0x12, 0x34, 0x56, 0x78]


    v = bs.pop_bits(4)
    assert v == 0x5

    v = bs.pop_bits(5)
    assert v == 0xa

    v = bs.pop_bits(7)
    assert v == 0x55

    v = bs.pop_bits(6)
    assert v == 0x07

    v = bs.pop_bits(2)
    assert v == 0x0

    v = bs.pop_bits(20)
    assert v == 0x12345

    v = bs.pop_bits(12)
    assert v == 0x678


    assert list(bs.get_bytes()) == []

    for i in range(8):
        bs.push_bits(0,1)
        bs.push_bits(1,1)
    assert list(bs.get_bytes()) == [0x55, 0x55]

    bs.push_bits(0x07, 6)
    assert bs.get_bytes() is None

    bs.flush()
    assert list(bs.get_bytes()) == [0x55, 0x55, 0x1c]

    bs.push_bits(0x12345678, 32)
    assert list(bs.get_bytes()) == [0x55, 0x55, 0x1c, 0x12, 0x34, 0x56, 0x78]


    v = bs.pop_bits(4)
    assert v == 0x5

    v = bs.pop_bits(5)
    assert v == 0xa

    v = bs.pop_bits(7)
    assert v == 0x55

    v = bs.pop_bits(6)
    assert v == 0x07

    v = bs.pop_bits(2)
    assert v == 0x0

    v = bs.pop_bits(20)
    assert v == 0x12345

    v = bs.pop_bits(12)
    assert v == 0x678


    assert list(bs.get_bytes()) == []

    
def test_ex2():
    bs = BitStream([0x12, 0x34])

    v = bs.pop_bits(4)
    assert v == 0x01
    v = bs.pop_bits(8)
    assert v == 0x23
    v = bs.pop_bits(4)
    assert v == 0x04

def test_ex3():
    bs = BitStream()
    for x in range(100000):
        n = x.bit_length()
        bs.push_bits(x, n)
    bs.flush()
    for x in range(100000):
        n = x.bit_length()
        y = bs.pop_bits(n)
        assert x == y
