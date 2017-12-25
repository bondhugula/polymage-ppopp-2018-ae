import numpy as np
import time
import sys

from __init__ import *

from init import init_all
from printer import print_header, print_config, print_line
from builder import create_lib,build_mfd
from exec_pipe import minifluxdiv
from app_tuner import auto_tune

app = "minifluxdiv"

def main():
    print_header()
    
    print("[main]: initializing...")
    print("")

    app_data = {}

    app_data['app'] = app
    app_data['app_name'] = app
    app_data['ROOT'] = ROOT

    g_sizes = [3, 5, 7]
    tile_sizes = [8, 16, 32, 64] #, 128, 256]

    init_all(app_data)
    print_config(app_data)
    if app_data['mode'] == 'tune+':
        for g_size in g_sizes:
            for t1 in tile_sizes:
                for t2 in tile_sizes:
                    create_lib(build_mfd, app, app_data, g_size, [1, t1, t2])
                    for t in range (0, 0):
                        print ("Running for iteration #", t)
   
    elif app_data['mode'] == 'tune':
        print("Tuning")
        auto_tune(app_data)
    else:
        create_lib(build_mfd, app, app_data)
        min_avg = 100000
        nsamples = 5
        for i in range (0, nsamples):
            min_avg = min (min_avg, minifluxdiv(app_data))
        print ("[main] Minimum of averaged times across ", nsamples,
                "samples: ", min_avg, " ms")

    return

main()
