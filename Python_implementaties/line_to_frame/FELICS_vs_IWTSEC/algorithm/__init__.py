import sys

import FELICS_vs_IWTSEC.algorithm.benchmark as benchmark
import FELICS_vs_IWTSEC.algorithm.cortex as cortex

__version__ = "0.1.0"


def run():
    args = sys.argv
    if len(args) == 1 or args[1] == "benchmark":
        benchmark.run(args)
    elif args[1] == "cortex":
        cortex.run(args)
