import math

class GolombRiceCode(object):
    def float_to_binary_fraction(value, precision_bits):
        integer_part = int(value)
        fractional_part = value - integer_part
        binary_integer_part = format(integer_part, 'b')
        
        binary_fraction_part = ""
        for _ in range(precision_bits):
            fractional_part *= 2
            bit = int(fractional_part)
            binary_fraction_part += str(bit)
            fractional_part -= bit
        
        return binary_integer_part + "." + binary_fraction_part

    def binary_fraction_to_float(binary_fraction):
        integer_part, fraction_part = binary_fraction.split('.')
        integer_value = int(integer_part, 2)
        fractional_value = 0.0
        for i, bit in enumerate(fraction_part):
            fractional_value += int(bit) * 2**(-i - 1)
        
        return integer_value + fractional_value
    

    def encode(self, n, m):
        precision_bits = 16
        quotient = n // m
        remainder = n % m
        q_len = int(quotient) + 1
        unary_code = '1' * q_len + '0'
        binary_fraction = GolombRiceCode.float_to_binary_fraction(remainder, precision_bits)
        byte_arr = unary_code + binary_fraction
        return byte_arr
    
    def decode(self, byte_arr, m):
        precision_bits = 16
        unary_len = byte_arr.find('0') + 1
        quotient = unary_len - 1
        binary_fraction = byte_arr[unary_len:unary_len + precision_bits + int(math.ceil(math.log2(m)))]
        remainder = GolombRiceCode.binary_fraction_to_float(binary_fraction)
        n = quotient * m + remainder
        return n
    
    