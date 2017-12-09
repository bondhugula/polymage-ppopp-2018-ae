import numpy as np
import time
import sys

from __init__ import *

from init import init_all
from printer import print_header, print_config, print_line
from builder import create_lib,build_interpolate
from exec_pipe import interpolate
#from app_tuner import auto_tune

app = "multiscale_interpolate"

def main():
    print_header()

    app_data = {}
    app_data['app'] = app
    app_data['ROOT'] = ROOT

    init_all(app_data)
    print_config(app_data)

    if app_data['mode'] == 'tune+':
        for g_size in [3, 5, 7, 9, 11, 200]:
            for t1 in [8, 16, 32, 64, 128, 256]:
                for t2 in [8, 16, 32, 64, 128, 256]:
                    create_lib(build_interpolate, app, app_data, g_size, [1, t1, t2])
                    for t in range (0, 0):
                        print ("Running for iteration #", t)
                        interpolate(app_data)
    elif app_data['mode'] == 'tune':
        pass
    else:
        create_lib(build_interpolate, app, app_data)
        _min_time = 10000
        outer_runs = 5
        for t in range(0, outer_runs):
            min_avg = min (min_avg, interpolate(app_data))
        
        print ("[main] Minimum of averaged times across ", outer_runs,
                "runs: ", min_avg, " ms")

    return

main()
