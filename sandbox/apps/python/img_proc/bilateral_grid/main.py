import numpy as np
import time
import sys
import os

from __init__ import *

from init import init_all
from printer import print_header, print_config, print_line
from builder import create_lib,build_bilateral
from exec_pipe import bilateralgrid
#from app_tuner import auto_tune

app = "bilateral"

def main():
    print_header()

    app_data = {}
    app_data['app'] = app
    app_data['ROOT'] = ROOT

    init_all(app_data)
    print_config(app_data)

    if app_data['mode'] == 'tune+':
        for g_size in [3, 5, 7]:
            for t1 in [8, 16, 32, 64, 128, 256]:
                for t2 in [8, 16, 32, 64, 128, 256]:
                    create_lib(build_bilateral, app, app_data, g_size, [1, t1, t2])
                    for t in range (0, 0):
                        print ("Running for iteration #", t)
                        bilateralgrid(app_data)
    elif app_data['mode'] == 'tune':
        pass
    else:
        create_lib(build_bilateral, app, app_data)
        min_avg = 10000
        # input ("wait to run amplxe " + str(os.getpid()))
        # input ("wwww")
        nsamples = 5
        for t in range(0, nsamples):
            min_avg = min (min_avg, bilateralgrid(app_data))
        print ("[main] Minimum of averaged times across ", nsamples,
                "samples: ", min_avg, " ms")

    return

main()
