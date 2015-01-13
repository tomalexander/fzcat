#!/bin/python3
#
import os
import time
import sys
import hashlib
from subprocess import Popen, PIPE

def dprint(*args):
    """Print to both stdout and stderr at the same time to allow for stdout redirection while still seeing the output in real time, rather than buffered like with tee"""
    print(*args)
    print(*args, file=sys.stderr
)
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

def cat_command(cmd):
    def run():
        m = hashlib.md5()
        command = [cmd]
        command.extend(all_files)
        print(command)
        p = Popen(command, stdout=PIPE, stdin=None, stderr=None)
        while True:
            inp = p.stdout.read(8192)
            if len(inp) == 0:
                return m.hexdigest();
            m.update(inp)
    return run

fzcat = cat_command("./fzcat")
zcat = cat_command("/bin/zcat")

dprint("zcat", zcat())

dprint("fzcat", fzcat())
