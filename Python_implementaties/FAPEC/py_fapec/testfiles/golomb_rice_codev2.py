import numpy as np
import tensorflow as tf
from tensorflow import math

class GolombRiceCode(object):  # kunnen we dit lower-level schrijven?
    def encode(self, n, m):  # with n the number to be encoded and parameter m
        q = tf.floor(n/m)
        r = tf.math.floormod(n, m)

        print(tf.dtypes.DType(q))
        quotient_code = tf.zeros(q)
        tf.append(quotient_code, 1)
        
        b = tf.floor(tf.log2(m))
        if r < 2**(b+1)-m :
            remainder_code = bin(r)
        else:
            remainder_code = bin(r+ 2**(b+1)-m)
        
        golomb_code = quotient_code + remainder_code

        return golomb_code
    
    def decode(arr, m):
        code = tf.array(arr)
        q = tf.nonzero(code)[0]  # geeft (normaalgezien) de hoeveelheid nullen aan het begin

        b = tf.floor(tf.log2(m))
        r_ = code[q+1:]
        if r_ < 2**(b+1)-m:
            r = r_
        else:
            r = r_ - 2**(b+1)+m
        
        n = q*m + r

        return n

