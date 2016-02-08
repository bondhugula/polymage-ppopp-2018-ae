import optparse
import sys

def parse_args():
    print("KUMUDHA: Inside Arg parser")
    parser = optparse.OptionParser()

    help_str = \
        '"new" : from scratch | "existing" : compile and run |  "ready" : just run'
    parser.add_option('-m', '--mode',
                       type='choice',
                       action='store',
                       dest='mode',
                       choices=['new', 'existing', 'ready', 'tune'],
                       default=['new'],
                       help=help_str)

    parser.add_option('-i', '--img',
                       action='store',
                       dest='img_file',
                       help='input image file path',)

    parser.add_option('-x', '--rows',
                       action='store',
                       dest='rows',
                       default=0,
                       help='number of rows of image ROI',)

    parser.add_option('-y', '--cols',
                       action='store',
                       dest='cols',
                       default=0,
                       help='number of cols of image ROI',)

    parser.add_option('-n', '--runs',
                       action='store',
                       dest='runs',
                       default=1,
                       help='number of runs',)

    parser.add_option('-t', '--timer',
                       action='store_true',
                       dest='timer',
                       default=False,
                       help='True : report execution time, \
                             False: do not collect timing info',)

    parser.add_option('-d', '--display',
                       action='store_true',
                       dest='display',
                       default=False,
                       help='display output image',)

    parser.add_option('--pool_alloc',
                       action='store_true',
                       dest='pool_alloc',
                       default=False,
                       help='True : Use a pool of memory allocations, \
                             False: generate simple malloc function call',)

    (options, args) = parser.parse_args()

    return options
