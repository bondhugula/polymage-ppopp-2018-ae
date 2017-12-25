# 
# Contributed by: Eddie Davis <eddiedavis@u.boisestate.edu>
#
from __init__ import *

import sys
import subprocess
import numpy as np

sys.path.insert(0, ROOT)

from compiler import *
from constructs import *

# PolyMage Specification
# ======================

def minifluxdiv(pipe_data):
    # Params
    B = Parameter(Int, "B")         # numBox
    N = Parameter(Int, "N")         # numCell

    threshold = Parameter(Float, "threshold")
    weight = Parameter(Float, "weight")
 
    pipe_data['B'] = B
    pipe_data['N'] = N
    pipe_data['threshold'] = threshold
    pipe_data['weight'] = weight

    # MFD Constants
    C = 5                   # components
    G = 2                   # ghosts
    fac1 = 1.0 / 12.0       # factor1
    fac2 = 2.0              # factor2

    # Vars
    x = Variable(Int, "x")
    y = Variable(Int, "y")
    z = Variable(Int, "z")
    c = Variable(Int, "c")

    # Input Image
    box = Image(Double, "box", [C, N+2*G, N+2*G, N+2*G])

    # Intervals
    ci = Interval(Int, 0, C-1)

    xz = Interval(Int, 0, N-1)
    xy = Interval(Int, 0, N-1)
    xx = Interval(Int, 0, N)

    yz = Interval(Int, 0, N-1)
    yy = Interval(Int, 0, N)
    yx = Interval(Int, 0, N-1)

    zz = Interval(Int, 0, N)
    zy = Interval(Int, 0, N-1)
    zx = Interval(Int, 0, N-1)

    dz = Interval(Int, 0, N-1)
    dy = Interval(Int, 0, N-1)
    dx = Interval(Int, 0, N-1)

    #####################################################################################
    # MiniFluxDiv Functions
    #####################################################################################

    f1x = Function(([c, z, y, x], [ci, xz, xy, xx]), Double, "f1x")
    f1x.defn = [fac1 * (box(c, z+G, y+G, x+G-2)
                     + 7.0 * (box(c, z+G, y+G, x+G-1) \
                     + box(c, z+G, y+G, x+G)) \
                     + box(c, z+G, y+G, x+G+1))]

    f2x = Function(([c, z, y, x], [ci, xz, xy, xx]), Double, "f2x")
    f2x.defn = [fac2 * (f1x(c, z, y, x) * f1x(2, z, y, x))]

    diffx = Function(([c, z, y, x], [ci, dz, dy, dx]), Double, "diffx")
    diffx.defn = [f2x(c, z, y, x + 1) - f2x(c, z, y, x)]

    f1y = Function(([c, z, y, x], [ci, yz, yy, yx]), Double, "f1y")
    f1y.defn = [fac1 * (box(c, z+G, y+G-2, x+G)
                     + 7.0 * (box(c, z+G, y+G-1, x+G)
                     + box(c, z+G, y+G, x+G))
                     + box(c, z+G, y+G+1, x+G))]

    f2y = Function(([c, z, y, x], [ci, yz, yy, yx]), Double, "f2y")
    f2y.defn = [fac2 * (f1y(c, z, y, x) * f1y(3, z, y, x))]

    diffy = Function(([c, z, y, x], [ci, dz, dy, dx]), Double, "diffy")
    diffy.defn = [f2y(c, z, y+1, x) - f2y(c, z, y, x)]

    f1z = Function(([c, z, y, x], [ci, zz, zy, zx]), Double, "f1z")
    f1z.defn = [fac1 * (box(c, z+G-2, y+G, x+G)
                     + 7.0 * (box(c, z+G-1, y+G, x+G)
                     + box(c, z+G, y+G, x+G))
                     + box(c, z+G+1, y+G, x+G))]

    f2z = Function(([c, z, y, x], [ci, zz, zy, zx]), Double, "f2z")
    f2z.defn = [fac2 * (f1z(c, z, y, x) * f1z(4, z, y, x))]

    diffz = Function(([c, z, y, x], [ci, dz, dy, dx]), Double, "diffz")
    diffz.defn = [f2z(c, z+1, y, x) - f2z(c, z, y, x)]

    sums = Function(([c, z, y, x], [ci, dz, dy, dx]), Double, "sums")
    sums.defn = [diffz(c, z, y, x) + diffy(c, z, y, x) +
                 diffx(c, z, y, x) + box(c, z+G, y+G, x+G)]

    #####################################################################################
    return sums, []
# END
