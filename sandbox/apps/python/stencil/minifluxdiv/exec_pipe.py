from __init__ import *

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
    boxes = app_data['boxes']
    cells = app_data['cells']
    threshold = app_data['threshold']
    weight = app_data['weight']

    mfd_data = app_data['mfd_data']
    IN = mfd_data['IN']
    OUT = mfd_data['OUT']

    # lib function name
    func_name = 'pipeline_'+app_data['app_name']
    pipe_func = app_data[func_name]
 
    # '''
    # #row_base = (rows - rowbase)/2
    # #col_base = (cols - colbase)/2
    # image_region = image[row_base:row_base+rows,col_base:col_base+cols]
    #
    # # create ghost zones
    # image_ghost = np.zeros((rows+4, cols+4, 3), image_region.dtype)
    # image_ghost[2:rows+2, 2:cols+2, 0:3] = image_region[0:rows, 0:cols, 0:3]
    #
    # # convert input image to floating point
    # image_f = np.float32(image_ghost) / 255.0
    #
    # # move colour dimension outside
    # image_f_flip = np.rollaxis(image_f, 2).ravel()
    #
    # # result array
    # res = np.empty((3, rows, cols), np.float32)
    # '''

    # lib function args
    pipe_args = []
    pipe_args += [ctypes.c_int(cells)]                  # N
    pipe_args += [ctypes.c_void_p(IN.ctypes.data)]      # box_void_arg
    pipe_args += [ctypes.c_void_p(OUT.ctypes.data)]     # sums_void_arg

    # call lib function
    pipe_func(*pipe_args)
    
    return

def minifluxdiv(app_data):
    it  = 0
    app_args = app_data['app_args']
   
    runs = int(app_args.runs)
    
    #input ("attach to amplxe-cl")
    #input ("attach to amplxe-cl11111")
    avg = 0
    while it < runs :
        t1 = time.time()
        call_pipe(app_data)
        t2 = time.time()

        time_taken = float(t2) - float(t1)
        avg += time_taken
        #print("")
        #print("[exec_pipe] : time taken to execute = ", (time_taken * 1000), " ms")
    
        it += 1
   
    print ("[exec_pipe] Average time across ", runs, " runs: ", avg/runs*1000, " ms")

    return avg/runs*1000
