def encode(x, k):
    mask = (1 << k) - 1
    l = x & mask
    h = x >> k
    assert h < 61
    ones = (1 << h) - 1
    bits = (ones << (k + 1)) | l
    n = h + 1 + k
    return bits, n

def push_bits(bs, x, k):
    mask = (1 << k) - 1
    l = x & mask
    h = x >> k
    for i in range(h):
        bs.push_bits(1, 1)
    bs.push_bits(0, 1)
    bs.push_bits(l, k)


def pop_bits(bs, k):
    h = 0
    while bs.pop_bits(1) == 1:
        h += 1
    l = bs.pop_bits(k)
    return (h << k) | l
