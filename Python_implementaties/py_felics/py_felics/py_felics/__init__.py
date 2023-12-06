import sys

import benchmark as benchmark
import cortex as cortex

__version__ = '0.1.0'


def run():
    print("running py_felics")
    args = sys.argv
    print(args[0])
    if args[1] == "benchmark":
        benchmark.run(args)
    elif args[1] == "cortex":
        cortex.run(args)


if __name__ == "__main__":
    run()
