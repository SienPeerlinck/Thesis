import sys

import benchmark as benchmark
import benchmarkv2


__version__ = '0.1.0'


def run():
    print("running py_felics")
    args = sys.argv
    print(args[0])
    if args[1] == "benchmark":
        benchmark.run(args)
    if args[1] == "benchmarkv2":
        benchmarkv2.run(args)


if __name__ == "__main__":
    run()
