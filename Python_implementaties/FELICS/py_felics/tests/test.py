import numpy as np
from collections import deque




def push(x):
    bitstream = deque([])
    d = x.bit_length()
    # k = self.__context.estimate_k(d)
    # self.__k_list.append(k)
    k=12
    # self.__context.update(x, delta, k)
    b = x.bit_length() - 1
    if b < k:
        b = k
        u = 0
    else:
        u = b - k + 1

    # Unary code of u
    for i in range(u):
        bitstream.append(1)
    bitstream.append(0)

    # b least significant bits of x
    mask = (1 << b) - 1
    bitstream.extend(int(bit) for bit in format(x & mask, '0' + str(b) + 'b'))
    return bitstream

def pop(bitstream):
    print(bitstream)
    x = deque([])
    # k = self.__context.estimate_k()
    # k = self.__k_list.popleft()
    # except IndexError: k=12
    k=12
    u = 0
    # while self.__bs.pop_bits(1) == 1:
    #     u += 1
    # if u == 0:
    #     b = k
    #     x = 0
    # else:
    #     b = u + k - 1
    #     x = 1
    # x <<= b
    # x |= self.__bs.pop_bits(b)
    while bitstream.popleft() == 1:
        u += 1
    if u == 0:
        b = k
        x.append(0)
    else:
        b = u + k - 1
        x.append(1)
    
    while b>0:
        b-=1
        x.append(bitstream.popleft())
    # self.__context.update(x, delta, k)
    return x



x = 603781
print(x.bit_length())
y = push(x)
print(len(y))
print(push(x))
z = pop(y)


print (bin(x))
print(z)