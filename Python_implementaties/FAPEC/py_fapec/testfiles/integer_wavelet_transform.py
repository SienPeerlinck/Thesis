# https://github.com/christofer-f/IntegerWaveletTransform/blob/main/iwt.py
import numpy as np
import random

def iwt53(c):    
    s = c[0::2]
    d = c[1::2]
    l = len(s)

    a = d[0:l-1] - np.floor(0.5*(s[0:l-1]+s[1:l])) 
    b = d[l-1] - s[l-1] 
    d_ = np.concatenate((a, b), axis=None)   

    a = s[0] + np.floor(0.5*d[0] + 0.5)
    b = s[1:l] + np.floor(0.25*(d[1:l] + d[0:l-1]) + 0.5)
    s_ = np.concatenate((a, b), axis=None)

    c2 = np.column_stack((s_,d_)).ravel()
    
    return c2


def iiwt53(x):
    s = x[0::2]
    d = x[1::2]
    l = len(s)

    a = s[0] - np.floor(0.5*d[0] + 0.5)
    b = s[1:l] - np.floor(0.25*(d[1:l] + d[0:l-1]) + 0.5)
    s_ = np.concatenate((a, b), axis=None)

    a = d[0:l-1] + np.floor(0.5*(s[0:l-1]+s[1:l])) 
    b = d[l-1] + s[l-1] 
    d_ = np.concatenate((a, b), axis=None)   

    c2 = np.column_stack((s_,d_)).ravel()

    return c2


