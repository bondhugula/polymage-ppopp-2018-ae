import optparse

parser = optparse.OptionParser()

# Storage Optimization takes care of reusing arrays associated with different
# Functions. Enabling this optimization leads to reuse of arrays after a point
# in the program where the function output is not live.
help_str = 'True : Optimize for storage, by reusing arrays if possible, \
            False: Create separate array for each Function'
parser.add_option('--optimize-storage',
                  action='store_true',
                  dest='optimize_storage',
                  default=False,
                  help=help_str)

# Flattening of scratchpads to a single dimension
# This is helpful for storage optimization, if enabled, since scratchpads are
# of constant size, and sizes across individual dimensions do not matter once
# the array is linearized.
help_str = 'True : Linearize / flatten const scratchpads to one dimension, \
            False: Create multi-dimensional scratchpads'
parser.add_option('--flatten_scratchpad',
                  action='store_true',
                  dest='flatten_scratchpad',
                  default=False,
                  help=help_str)

# Early freeing of the arrays results in lesser memory footprint, by throwing
# away all unwanted arrays from the memory as soon as possible.
help_str =  'True : Free the arrays not in use as soon as possible, \
             False: Procrastinated freeing of arrays'
parser.add_option('--early-free',
                  action='store_true',
                  dest='early_free',
                  default=False,
                  help=help_str)

# Use a pool of memory, so that allocation from pool can return the pointer to
# an already allocated array, thus resulting in avoiding frequent malloc()
# calls across allocations / iterations of program
parser.add_option('--pool-alloc',
                  action='store_true',
                  dest='pool_alloc',
                  default=False,
                  help='True : Use a pool of memory allocations, \
                        False: generate simple malloc function call')

# To extract multidimensional parallelism, PolyMage uses OpenMP's collapse
# clause on perfectly nested parallel loops. Enabling this optimization results
# in generation of omp pragma with "collapse". However, parallel loop nests
# that are not perfectly nested remain unaffected.
parser.add_option('--multipar',
                  action='store_true',
                  dest='multipar',
                  default=False,
                  help='True : Mark omp \'collapse\' directive if possible, \
                        False: Simply mark \'omp parallel for\'')

#To enable automatic inlining
parser.add_option('--inline',
                  action='store_true',
                  dest='inline',
                  default=False,
                  help='True : Enable Inlining, \
                        False: Disable Inlining')

#To enable automatic inlining
parser.add_option('--multi-level-tiling',
                  action='store_true',
                  dest='multi_level_tiling',
                  default=False,
                  help='True : Enable Multi Level L2 and L1 Tiling, \
                        False: Disable Multi Level L2 and L1 Tiling')

# to enable DP-based fusion
parser.add_option('--dpfuse',
                  action='store_true',
                  dest='dpfuse',
                  default=False,
                  help='True : Enable DP-based fusion, \
                        False: Use a greedy fusion heuristic')

parser.add_option('--logdpchoices',
                  action='store_true',
                  dest='logdpchoices',
                  default=False,
                  help='True: Print the number of dp choices, \
                        False: don\'t')
parser.add_option('--logmaxchildren',
                  action='store',
                  dest='logmaxchildren',
                  default=3,
                  help='Set the max children in first iteration of Bounded Grouping Algorithm')
      
