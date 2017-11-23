import numpy as np
import time
import sys

from __init__ import *

from init import init_all
from printer import print_header, print_config, print_line
from builder import create_lib,build_test_fusion
from exec_pipe import test_fusion_pipe
from app_tuner import auto_tune

app = "test_node_fusion"

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
                    create_lib(build_test_fusion, app, app_data, g_size, [t1, t2])
                    for t in range (0, 0):
                        print ("Running for iteration #", t)
                        test_fusion_pipe(app_data)
    elif app_data['mode'] == 'tune':
        pass
    else:
        create_lib(build_test_fusion, app, app_data)
        _m = 10000000
        for i in range (0,5):
            _m = min (_m, test_fusion_pipe(app_data))
        print ("min time is ", _m)
    return

main()
