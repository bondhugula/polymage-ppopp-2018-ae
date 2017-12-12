from __init__ import *

import sys
import os.path
from PIL import Image
import numpy as np
from arg_parser import parse_args

from printer import print_header, print_usage, print_line

sys.path.insert(0, ROOT)
from utils import *

def init_images(app_data):
    print("[init.py] : initializing images...")

    app_args = app_data['app_args']

    off_left = 24
    total_pad = 56

    # input image: 
    img_path = app_args.img_file
    img1 = Image.open(img_path)  #.convert('1'))
    print ("size ", img1.size)
    img = np.array(img1)
    c = 0
    rows, cols = img.shape
  
    rowdiff = 2560
    coldiff = 1536

    row_base = int((rows-rowdiff)/2)
    col_base = int((cols-coldiff)/2)
    image_region = img[row_base:row_base+rows, \
                     col_base:col_base+cols]
    
    # create ghost zone and copy image roi
    image_ghost = \
        np.empty((rows+total_pad, cols+total_pad), image_region.dtype)
    
    image_ghost[off_left:rows+off_left, off_left:cols+off_left] = \
        np.array(image_region[0:rows, 0:cols], image_region.dtype)
    
    # clamp the boundary portions
    image_clamp(image_region, image_ghost, rows, cols, 0, \
                image_region.dtype, 1, off_left, total_pad)

    image_rgb = Image.fromarray(image_ghost)
    image1 = image_rgb.convert('1')

    pixels = image1.getdata()
    data = np.reshape(pixels,image1.size)
    OUT = np.zeros((rows, cols), np.float32).ravel()
    img_data = {}
    img_data['IN'] = data
    img_data['OUT'] = OUT

    app_data['img_data'] = img_data
    app_data['rows'] = rows
    app_data['cols'] = cols

    app_data['rowdiff'] = rowdiff
    app_data['coldiff'] = coldiff

    return

def get_input(app_data):
    # parse the command-line arguments
    app_args = parse_args()
    app_data['app_args'] = app_args

    app_data['mode'] = app_args.mode
    app_data['runs'] = int(app_args.runs)
    app_data['graph_gen'] = bool(app_args.graph_gen)
    app_data['timer'] = app_args.timer

    # storage optimization
    app_data['optimize_storage'] = bool(app_args.optimize_storage)
    # early freeing of allocated arrays
    app_data['early_free'] = bool(app_args.early_free)
    # pool allocate option
    app_data['pool_alloc'] = bool(app_args.pool_alloc)
    app_data['multi-level-tiling'] = bool(app_args.multi_level_tiling)
    app_data['dpfuse'] = bool(app_args.dpfuse)
    app_data['logdpchoices'] = bool(app_args.logdpchoices)
    app_data['logmaxchildren'] = int(app_args.logmaxchildren)
    
    return

def init_all(app_data):
    pipe_data = {}
    app_data['pipe_data'] = pipe_data

    get_input(app_data)

    init_images(app_data)

    return

