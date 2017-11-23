import numpy as np
import time
import sys

from __init__ import *

from init import init_all
from printer import print_header, print_config, print_line
from builder import create_lib,build_unsharp
from exec_pipe import unsharp_mask
from app_tuner import auto_tune

app = "unsharp_mask"

def main():
    print_header()
    
    print("[main]: initializing...")
    print("")

    app_data = {}

    app_data['app'] = app
    app_data['app_name'] = app
    app_data['ROOT'] = ROOT

    init_all(app_data)
    print_config(app_data)
    if app_data['mode'] == 'tune+':
        for g_size in [3, 5, 7]:
            for t1 in [8, 16, 32, 64, 128, 256]:
                for t2 in [8, 16, 32, 64, 128, 256]:
                    create_lib(build_unsharp, app, app_data, g_size, [1, t1, t2])
                    for t in range (0, 5):
                        print ("Running for iteration #", t)
   
    elif app_data['mode'] == 'tune':
        print("Tuning")
        auto_tune(app_data)
    else:
        create_lib(build_unsharp, app, app_data)
        min_avg = 100000
        for i in range (0, 5):
            min_avg = min (min_avg, unsharp_mask(app_data))
        
        print ("minimum average ", min_avg)

    return

main()
