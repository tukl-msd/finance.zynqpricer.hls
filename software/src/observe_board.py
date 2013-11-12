"""
Copyright (C) 2013 University of Kaiserslautern
Microelectronic Systems Design Research Group

Christian Brugger (brugger@eit.uni-kl.de)
11. November 2013
"""

import sys
import subprocess
import shlex
import time
import json
import threading


remote_encoding = 'latin1'

CMD = ("sudo taskset -c 1 software/bin/run_heston -sl -acc "
        "software/parameters/params_zynq_demo_acc.json "
        "bitstream/heston_sl_6x.json -observe")


def line_reader(p):
    line = []
    while p.poll() is None:
        r = p.stdout.read(1).decode(remote_encoding)
        if r == '\n':
            yield ''.join(line)
            line = []
        else:
            line.append(r)
    yield ''.join(line) + p.stdout.read().decode(remote_encoding)


class RemoteObserver(threading.Thread):
    def __init__(self, pty_cmd, root_dir):
        threading.Thread.__init__(self)
        self.pty_cmd = pty_cmd
        self.root_dir = root_dir
        self._p = None


    def _send_cmd(self, msg):
        self._p.stdin.write((msg + '\n').encode(remote_encoding))


    def run(self):
        self._p = subprocess.Popen(shlex.split(self.pty_cmd), 
                stdin=subprocess.PIPE, stdout=subprocess.PIPE, 
                stderr=sys.stderr)
        reader = line_reader(self._p)
        # wait for welcome message
        for line in reader:
            if line.startswith('Last login:'):
                break

        self._send_cmd("cd {}".format(shlex.quote(self.root_dir)))
        self._send_cmd(CMD)
        self._send_cmd("exit")
    
        for line in reader:
            try:
                data = json.loads(line)
            except ValueError:
                pass
            else:
                if data["__class__"] == "observer":
                    print(data)


def main():
    if len(sys.argv) != 3:
        print("Usage: {} <pty_cmd> <root_dir>".format(argv[0]))
        sys.exit(1)
    pty_cmd = sys.argv[1]
    root_dir = sys.argv[2]

    observer = RemoteObserver(pty_cmd, root_dir)
    observer.start()
    observer.join()


if __name__ == "__main__":
    main()