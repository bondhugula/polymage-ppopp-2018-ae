# Making things compatible for python 3
# Yet to figure out how to make range like xrange
from __future__ import absolute_import, division, print_function

from fractions import Fraction
import sys
sys.path.insert(0, '../')

from compiler import *
from constructs import *

def _test_math():

    R = Parameter(Int, "R")
    C = Parameter(Int, "C")
    x = Variable(Int, "x")
    y = Variable(Int, "y")

    row = Interval(Int, 0, R-1)
    col = Interval(Int, 0, C-1)

    img1 = Image(Float, "img1", [R, C])
    img2 = Image(Float, "img2", [R, C])

    cond = Condition(x, '>=', 0) & Condition(x, '<=', R-1) & \
           Condition(y, '<=', C-1) & Condition(y, '>=', 0)

    # sin(image1)
    sin = Function(([x, y], [row, col]), Float, "_sin")
    sin.defn = [ Case(cond, Sin(img1(x, y))) ]
    # cos(image2)
    cos = Function(([x, y], [row, col]), Float, "_cos")
    cos.defn = [ Case(cond, Cos(img2(x, y))) ]

    # max(sin, cos)
    imax = Function(([x, y], [row, col]), Float, "_max")
    imax.defn = [ Case(cond, Max(sin(x, y), cos(x, y))) ]
    # min(sin, cos)
    imin = Function(([x, y], [row, col]), Float, "_min")
    imin.defn = [ Case(cond, Min(sin(x, y), cos(x, y))) ]

    # max^min
    powf = Function(([x, y], [row, col]), Float, "_powf")
    powf.defn = [ Case(cond, Powf(imax(x, y), imin(x, y))) ]
    # e^powf
    exp = Function(([x, y], [row, col]), Float, "_exp")
    exp.defn = [ Case(cond, Exp(powf(x, y))) ]

    # sqrtf(image1)
    sqrtf = Function(([x, y], [row, col]), Float, "_sqrtf1")
    sqrtf.defn = [ Case(cond, Sqrtf(img1(x, y))) ]

    # cast<int>(image2)
    cast = Function(([x, y], [row, col]), Int, "_cast")
    cast.defn = [ Case(cond, Cast(Int, img2(x, y))) ]
    # sqrt(cast)
    sqrt = Function(([x, y], [row, col]), Int, "_sqrt2")
    sqrt.defn = [ Case(cond, Sqrt(cast(x, y))) ]

    # a test for implicit typecasting as well
    # | sqrtf - sqrt |
    iabs = Function(([x, y], [row, col]), Float, "_abs")
    iabs.defn = [ Case(cond, Abs(sqrtf(x, y) - sqrt(x, y))) ]

    # pow(abs, 3)
    ipow = Function(([x, y], [row, col]), Float, "_pow")
    ipow.defn = [ Case(cond, Pow(iabs(x, y), 3)) ]

    # image1 > image2 ? pow : exp
    select_cond = Condition(img1(x, y), '>', img2(x, y))
    select = Function(([x, y], [row, col]), Float, "_select")
    select.defn = [ Case(cond, Select(select_cond, ipow(x, y), exp(x, y))) ]

    pipeline = buildPipeline([select],
                             pipe_name="aimless")

    '''
    filename = 'aimless.dot'
    pipeline.originalGraph.write(filename)
    
    filename = 'aimless_groupeed.dot'
    g = pipeline.drawPipelineGraph()
    g.write(filename)
    '''

    filename = 'aimless.cpp'
    c_file = open(filename, 'w')
    c_file.write(pipeline.generate_code().__str__())
    c_file.close()

