import array
import sys

if len(sys.argv) != 2:
    print("Argument Error !")
    sys.exit(-1)


with open(sys.argv[1], "rb") as fd:
    buf = fd.read()
    ws = array.array("I", buf)
    # ws.byteswap()
    print("#define FPGA_CONTENTS_N %d" % len(buf))
    print()
    print("static char fpga_contents[] = {")
    for w in ws:
        for _ in range(4):
            print("  %d," % (w & 0xff))
            w >>= 8
    print("};")


"""
with open(sys.argv[1], "rb") as fd:
    buf = fd.read()
    ws = array.array("I", buf)
    print("#define FPGA_CONTENTS_N %d" % len(buf))
    print()
    print("static char fpga_contents[] = {")
    for b in buf:
        print("  %d," % b)
    print("};")
"""
