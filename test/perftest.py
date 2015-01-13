#!/bin/python3
#
import os
import time
import sys
from subprocess import call, DEVNULL

def dprint(*args):
    """Print to both stdout and stderr at the same time to allow for stdout redirection while still seeing the output in real time, rather than buffered like with tee"""
    print(*args)
    print(*args, file=sys.stderr)

def get_files(p):
    if os.path.isfile(p):
        return [p]
    ret = []
    for root, dirs, files in os.walk(p):
        ret.extend([os.path.join(root, f) for f in files])
    return ret

all_files = []
for d in sys.argv[1:]:
    all_files.extend(get_files(d))

def cat():
    command = ["/bin/cat"]
    command.extend(all_files)
    start = time.time()
    call(command, stdout=DEVNULL)
    end = time.time()
    return end-start

def fzcat(buffer_size):
    command = ["./fzcat"]
    command.extend(all_files)
    start = time.time()
    call(command, stdout=DEVNULL)
    end = time.time()
    return end-start

def zcat():
    command = ["/bin/zcat"]
    command.extend(all_files)
    start = time.time()
    call(command, stdout=DEVNULL)
    end = time.time()
    return end-start

def pigz():
    command = ["/bin/pigz", "-dc"]
    command.extend(all_files)
    start = time.time()
    call(command, stdout=DEVNULL)
    end = time.time()
    return end-start

def parallel_zcat():
    command = ["/bin/parallel", "-k", "--no-notice", "/bin/zcat", ":::"]
    command.extend(all_files)
    start = time.time()
    call(command, stdout=DEVNULL)
    end = time.time()
    return end-start

def drop_cache():
    with open("/proc/sys/vm/drop_caches", "w") as f:
        f.write("3")

dprint("Warming up by catting all files to /dev/null")
cat()

dprint("zcat", zcat())

dprint("parallel_zcat", parallel_zcat())

dprint("pigz", pigz())

dprint("fzcat", fzcat())

dprint("Running again but dropping cache between runs")

drop_cache()
dprint("zcat", zcat())

drop_cache()
dprint("parallel_zcat", parallel_zcat())

drop_cache()
dprint("pigz", pigz())

drop_cache()
dprint("fzcat", fzcat())
