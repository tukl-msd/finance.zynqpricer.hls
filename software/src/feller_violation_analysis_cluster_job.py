"""
Copyright (C) 2013 University of Kaiserslautern
Microelectronic Systems Design Research Group

Christian Brugger (brugger@eit.uni-kl.de)
10. October 2013
"""

import platform
import sys
import json
import subprocess
import shlex

if platform.system() == "Windows":
    exe_path = "bin/eval_heston.exe"
    is_posix = False
elif platform.system() == "Linux":
    exe_path = "bin/eval_heston"
    is_posix = True
else:
    raise Exception("Unknown platform: " + platform.system())


if len(sys.argv) != 3:
    print("ERROR: usage: {} <params_file> <result_file>".format(sys.argv[0]))
    sys.exit(-1)
params_path = sys.argv[1]
result_path = sys.argv[2]


####

    
with open(params_path) as f:
    params = json.load(f)

cmd = "{} -ml {}".format(exe_path, params_path)

raw = subprocess.check_output(shlex.split(cmd, posix=is_posix))
print(raw.decode("utf-8"))
data = json.loads(raw.decode("utf-8"))

with open(result_path, 'wb') as f:
    f.write(raw)
    
#TODO(brugger): asian option

