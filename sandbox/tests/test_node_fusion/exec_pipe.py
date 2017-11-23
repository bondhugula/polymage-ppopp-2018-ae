import sys
import os
import ctypes
import numpy as np
import time

from printer import print_line

from compiler   import *
from constructs import *
from utils import *

def call_pipe(app_data):
    rows = app_data['rows']-2
    cols = app_data['cols']-2

    img_data = app_data['img_data']
    IN = img_data['IN']
    OUT = img_data['OUT']
    OUT1 = img_data['OUT1']

    # lib function name
    func_name = 'pipeline_'+app_data['app']
    pipe_func = app_data[func_name]

    # lib function args
    pipe_args = []
    pipe_args += [ctypes.c_int(cols)]
    pipe_args += [ctypes.c_int(rows)]
    pipe_args += [ctypes.c_void_p(IN.ctypes.data)]
    pipe_args += [ctypes.c_void_p(OUT.ctypes.data)]
    pipe_args += [ctypes.c_void_p(OUT1.ctypes.data)]

    # call lib function
    pipe_func(*pipe_args)
    
    return

def test_fusion_pipe(app_data):

    it  = 0
    app_args = app_data['app_args']
   
    runs = int(app_args.runs)
    timer = app_args.timer
    #input ("attach to amplxe-cl")
    #input ("attach to amplxe-cl11111")
    avg = 0
    while it < runs :
        t1 = time.time()
        call_pipe(app_data)
        t2 = time.time()

        time_taken = float(t2) - float(t1)
        avg += time_taken
        print("")
        print("[exec_pipe] : time taken to execute = ", (time_taken * 1000), " ms")

        it += 1

    print ("average time ", avg/runs*1000, " ms")
    return avg/runs*1000
