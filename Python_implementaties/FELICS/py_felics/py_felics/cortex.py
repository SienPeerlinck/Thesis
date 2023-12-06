from PIL import Image
from matplotlib import pyplot as plt
from serial import Serial
from serial.tools import list_ports

import py_felics.felics as felics
from py_felics.detector_sim import *
from py_felics.context import Context0

DEFAULT_SHAPE = (16, 384)
SAM71_SN = "ATML2407131800002413"

K_TABLE1 = [12 for _ in range(21)]
for i in range(15, 21):
    K_TABLE1[i] = 14

class Cortex(object):
    def __init__(self):
        ports = list_ports.comports()
        usb_interface = None
        for port in ports:
            if port.serial_number == SAM71_SN:
                usb_interface = port
        if usb_interface is None:
            raise RuntimeError("Serial-USB %s not found." % SAM71_SN)
            
        self.__ser_if = Serial(usb_interface.device, 500000, timeout = 0.5)

    def run(self, im, k_table):
        k_table = self.set_k_table(k_table)
        ctx = Context0(k_table)
        print("k table", list(k_table))

        (height, width) = im.shape
        # Header
        buf = bytes([width >> 4, ((width & 0xf) << 4) | height >> 8, height & 0xff])
        t = 0

        # Send Row 0
        row0 = im[0]
        cmd = b"row0 "
        for x in row0:
            cmd += b"%05x" % x
        cmd += b"\n"
        self.__ser_if.write(cmd)
        response = self.__ser_if.read(4000)
        if response.startswith(b"ok "):
            sr = response.split()
            t += int(sr[1])
            hex_row = str(sr[3], "utf8")
            buf += bytes.fromhex(hex_row)

        # Send Next Rows
        for i in range(1, height):
            print("%d/%d" % (i,height), end="\r")
            row = im[i]
            cmd = b"row "
            for x in row:
                cmd += b"%05x" % x
            cmd += b"\n"
            self.__ser_if.write(cmd)
            response = self.__ser_if.read(4000)
            if response.startswith(b"ok "):
                sr = response.split()
                t += int(sr[1])
                hex_row = str(sr[3], "utf8")
                buf += bytes.fromhex(hex_row)
        
        # Flush
        cmd = b"flush\n"
        self.__ser_if.write(cmd)
        response = self.__ser_if.read(4000)
        if response.startswith(b"ok "):
            sr = response.split()
            t += int(sr[1])
            if len(sr) == 4:
                hex_row = str(sr[3], "utf8")
                buf += bytes.fromhex(hex_row)

            im2 = felics.decode(buf, 20, ctx)
            return t, im2, len(buf)
        return None, None, None

    def set_k_table(self, k_table):
        cmd = b"k_table "
        for k in k_table:
            cmd += b"%02x" % k
        cmd += b"\n"
        self.__ser_if.write(cmd)
        print("k_table cmd", cmd)
        response = self.__ser_if.read(4000)
        print("  -> ", response)
        if response.startswith(b"ok "):
            sr = response.split()
            return bytes.fromhex(str(sr[1], "utf8"))
        return None
        
    def close(self):
         self.__ser_if.close()         


def ex1(args):
    width = int(args[2])
    height = int(args[3])
    assert width <= 400
    
    shape = (height, width)
    im = make_signal1(shape, 1000000)
    im += make_noise(shape, 10000)
    #im += 40000
    im = add_bad_pixels(im)
    im = add_vignetting(im)
    im = np.clip(im, 0, 0xfffff)
    im = im.astype(np.uint32)
    #print(im.shape, im.size)

    print("Started with %dx%d image" % im.shape)
    sam71 = Cortex()
    t, im2, len_buf = sam71.run(im, K_TABLE1)
    if t is None:
        print("Cortex error")
    else:
        ratio = len_buf / im2.size
        print("Verification", np.all(im == im2))
        fmt = "Compressed Size: %d bytes => %f bytes/pixel => %f bits/pixel"
        print(fmt % (len_buf, ratio, ratio*8))
        print("Compression Time: %d cycles => %f cycles/pixel" % (t, t/im2.size))
        plt.imshow(im2, cmap="gray", interpolation='nearest')
        plt.show()



    sam71.close()
 

def run(args):
    ex1(args)
