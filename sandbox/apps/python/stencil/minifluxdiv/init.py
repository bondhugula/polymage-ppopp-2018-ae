import sys
import os.path
from PIL import Image
import numpy as np
from arg_parser import parse_args

from printer import print_header, print_usage, print_line


def init_images(app_data):
    print("[init.py] : initializing boxes...")

    app_args = app_data['app_args']

    nBoxes = int(app_args.boxes)
    nCells = int(app_args.cells)
    nComp = 5
    nGhost = 2
    dx = 0.5
    fullCells = nCells + 2 * nGhost

    # Init numpy array...
    dtype = np.float64
    #data = np.ndarray((nBoxes, fullCells, fullCells, fullCells, nComp), dtype)
    data = np.ndarray((nComp, fullCells, fullCells, fullCells), dtype)
    output = np.ndarray((nComp, fullCells, fullCells, fullCells), dtype)

    #for b in range(nBoxes):
    for z in range(fullCells):
        for y in range(fullCells):
            for x in range(fullCells):
                sub = dx * (z + y + x - 6)
                data[0, z, y, x] = dx * ((z-2) + (y-2) + (x-2))
                data[1, z, y, x] = 1.0 + sub
                data[2, z, y, x] = 2.0 + sub
                data[3, z, y, x] = 3.0 + sub
                data[4, z, y, x] = 4.0 + sub
                for c in range(nComp):
                    output[c, z, y, x] = data[c, z, y, x]

    #output = np.zeros((nComp, nCells, nCells, nCells), dtype)

    mfd_data = {}
    mfd_data['IN'] = data
    mfd_data['OUT'] = output

    app_data['mfd_data'] = mfd_data
    app_data['boxes'] = nBoxes
    app_data['cells'] = nCells
    app_data['ghost'] = nGhost
    app_data['comp'] = nComp
    
    return

def get_input(app_data):
    app_args = parse_args()
    app_data['app_args'] = app_args

    app_data['mode'] = app_args.mode
    app_data['threshold'] = float(app_args.threshold)
    app_data['weight'] = int(app_args.weight)

    app_data['runs'] = int(app_args.runs)
    app_data['graph_gen'] = bool(app_args.graph_gen)
    app_data['timer'] = app_args.timer

    # storage optimization
    app_data['optimize_storage'] = bool(app_args.optimize_storage)
    # early freeing of allocated arrays
    app_data['early_free'] = bool(app_args.early_free)
    # pool allocate option
    app_data['pool_alloc'] = bool(app_args.pool_alloc)
    # inline
    app_data['inline'] = bool (app_args.inline)
    app_data['multi-level-tiling'] = bool(app_args.multi_level_tiling)
    app_data['dpfuse'] = bool(app_args.dpfuse)

    return

def init_all(app_data):
    pipe_data = {}
    app_data['pipe_data'] = pipe_data

    get_input(app_data)

    init_images(app_data)

    return

