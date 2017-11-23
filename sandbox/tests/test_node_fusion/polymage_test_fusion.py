from __init__ import *

import sys
import subprocess
import numpy as np
from fractions import Fraction

sys.path.insert(0, ROOT)

from compiler import *
from constructs import *

def test_fusion_pipe(pipe_data):

    R = Parameter(Int, "R")
    C = Parameter(Int, "C")
    x = Variable(Int, "x")
    y = Variable(Int, "y")

    pipe_data['R'] = R
    pipe_data['C'] = C

    row = Interval(Int, 0, R+1)
    col = Interval(Int, 0, C+1)

    cond = Condition(x, '>=', 1) & Condition(x, '<=', R) & \
           Condition(y, '<=', C) & Condition(y, '>=', 1)

    img = Image(Float, "img", [R+2, C+2])

    Iy = Function(([x, y], [row, col]), Float, "Iy")
    Iy.defn = [ Case(cond, img(x-1, y-1)*(-1.0/12) + img(x-1, y+1)*(1.0/12) + \
                           img(x, y-1)*(-2.0/12) + img(x, y+1)*(2.0/12) + \
                           img(x+1, y-1)*(-1.0/12) + img(x+1, y+1)*(1.0/12)) ]

    Ix = Function(([x, y], [row, col]), Float, "Ix")
    Ix.defn = [ Case(cond, img(x-1, y-1)*(-1.0/12) + img(x+1, y-1)*(1.0/12) + \
                           img(x-1, y)*(-2.0/12) + img(x+1, y)*(2.0/12) + \
                           img(x-1, y+1)*(-1.0/12) + img(x+1, y+1)*(1.0/12)) ]
 
    return [Ix, Iy]
