#
# Copyright 2014-2016 Vinay Vasista, Ravi Teja Mullapudi, Uday Bondhugula,
# and others from Multicore Computing Lab, Department of Computer Science
# and Automation, Indian Institute of Science
#

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# 
#
# pipe.py : Intermediate representation of pipeline specification and driving
#           the optimization processes at a high level.
#

from __future__ import absolute_import, division, print_function

# More Python 3 vs 2 mojo
try:
    import queue
except ImportError:
    import Queue as queue

import pygraphviz as pgv
import targetc as genc
from functools import reduce

from grouping import *

from constructs import *
from expression import *
from codegen import *
from schedule import *
from poly_schedule import *
from align_scale import *
from poly import *
from bounds import *
from inline import *
from liveness import *
from storage_mapping import *

# LOG CONFIG #
pipe_logger = logging.getLogger("pipe.py")
pipe_logger.setLevel(logging.INFO)
LOG = pipe_logger.log

MACHINE_TYPE = ['personal', 'mcastle1', 'mcastle2', 'polymage'][3]
    
if (MACHINE_TYPE == 'personal'):
    #global IMAGE_ELEMENT_SIZE, L2_CACHE_SIZE, N_CORES, TILING_THRESHOLD, VECTOR_WIDTH_THRESHOLD
    #For Dekstop/Laptop Machine
    IMAGE_ELEMENT_SIZE = 4
    L2_CACHE_SIZE = int(256*1024/IMAGE_ELEMENT_SIZE)
    N_CORES = 4
    TILING_THRESHOLD = 1
    VECTOR_WIDTH_THRESHOLD = 16
elif (MACHINE_TYPE == 'mcastle1'): #Intel Haswell machine
    #global IMAGE_ELEMENT_SIZE, L2_CACHE_SIZE, N_CORES, TILING_THRESHOLD, VECTOR_WIDTH_THRESHOLD
    #For mcastle
    IMAGE_ELEMENT_SIZE = 4
    L2_CACHE_SIZE = int(512*1024/IMAGE_ELEMENT_SIZE)
    N_CORES = 16
    TILING_THRESHOLD = 1
    VECTOR_WIDTH_THRESHOLD = 64 #TODO: 
    L2_INNER_MOST_DIM_SIZE = 256
    L1_INNER_MOST_DIM_SIZE = 256
    L1_CACHE_SIZE = int(32*1024/IMAGE_ELEMENT_SIZE)
    OUTER_DIM_TILING_THRESH = 4
    THRESHOLD_TILE_SIZE = 4
elif (MACHINE_TYPE == 'polymage'): #Intel Haswell machine
    #global IMAGE_ELEMENT_SIZE, L2_CACHE_SIZE, N_CORES, TILING_THRESHOLD, VECTOR_WIDTH_THRESHOLD
    #For mcastle
    IMAGE_ELEMENT_SIZE = 4
    L2_CACHE_SIZE = int(256*1024/IMAGE_ELEMENT_SIZE)
    N_CORES = 16
    TILING_THRESHOLD = 1
    VECTOR_WIDTH_THRESHOLD = 64 #TODO: 
    L2_INNER_MOST_DIM_SIZE = 256
    L1_INNER_MOST_DIM_SIZE = 256
    L1_CACHE_SIZE = int(32*1024/IMAGE_ELEMENT_SIZE)
    OUTER_DIM_TILING_THRESH = 4
    THRESHOLD_TILE_SIZE = 4
elif (MACHINE_TYPE == 'mcastle2'): #AMD Opteron machine
    #global IMAGE_ELEMENT_SIZE, L2_CACHE_SIZE, N_CORES, TILING_THRESHOLD, VECTOR_WIDTH_THRESHOLD
    #For mcastle
    IMAGE_ELEMENT_SIZE = 4
    L2_CACHE_SIZE = int(1024*1024/IMAGE_ELEMENT_SIZE)
    #TODO: In case of multi level tiling L2_CACHE_SIZE must be set to 256KB
    #It is automatically set to 256KB in MachineInformation class
    N_CORES = 16
    TILING_THRESHOLD = 1
    VECTOR_WIDTH_THRESHOLD = 64 #TODO: 
    L2_INNER_MOST_DIM_SIZE = 256
    L1_INNER_MOST_DIM_SIZE = 128
    L1_CACHE_SIZE = int(16*1024/IMAGE_ELEMENT_SIZE)
    #TODO: For harris L1 Cache is 32KB, correct it in the implementation that
    #in multi-level tiling for harris (i.e. L1 tile in L2 tile) 
    #the size of _TL1_i0 should be atleast 5.
    OUTER_DIM_TILING_THRESH = 4
    THRESHOLD_TILE_SIZE = 4

def get_next_power_of_2 (num):
    num -= 1
    num |= num >> 1
    num |= num >> 2
    num |= num >> 4
    num |= num >> 8
    num |= num >> 16
    num |= num >> 32
    num += 1
    
    return num

def is_power_of_2 (num):
    return (num & (num-1)) == 0
    
def get_parents_from_func(func, non_image=True):
    refs = func.getObjects(Reference)
    # Filter out self and image references 
    if non_image:
        refs = [ ref for ref in refs if not ref.objectRef == func and \
                                     not isinstance(ref.objectRef, Image) ]
    else:
        refs = [ ref for ref in refs if not ref.objectRef == func ]

    return list(set([ref.objectRef for ref in refs]))

def get_funcs_and_dep_maps(outputs):
    """
    Find all the compute objects required for the outputs and
    also builds parent and children maps for the compute objects
    """
    funcs = []
    funcs_parents = {}
    funcs_children = {}
    # queue of compute objects
    q = queue.Queue()
    for func in outputs:
        q.put(func)
    while not q.empty():
        obj = q.get()
        parent_objs = get_parents_from_func(obj)
        if obj not in funcs:
            funcs.append(obj)
            funcs_parents[obj] = parent_objs
            for parobj in parent_objs:
                if parobj in funcs_children:
                    if obj not in funcs_children[parobj]:
                        funcs_children[parobj].append(obj)
                else:
                    funcs_children[parobj] = [obj]
            if len(parent_objs) != 0:
                for r in parent_objs:
                    q.put(r)

    for func in funcs:
        if func not in funcs_parents:
            funcs_parents[func] = []
        if func not in funcs_children:
            funcs_children[func] = []
    
    return funcs, funcs_parents, funcs_children

def get_funcs(outputs):
    """
    Find all the compute objects required for the outputs and
    also builds parent and children maps for the compute objects
    """
    funcs = []
    # queue of compute objects
    q = queue.Queue()
    for func in outputs:
        q.put(func)
    while not q.empty():
        obj = q.get()
        parent_objs = get_parents_from_func(obj, non_image=False)
        if obj not in funcs:
            funcs.append(obj)
            if len(parent_objs) != 0:
                for r in parent_objs:
                    q.put(r)

    return funcs


class ComputeTypes:
    FUNCTION = 1
    IMAGE = 2
    REDUCTION = 3
    TSTENCIL = 4


class ComputeObject:
    def __init__(self, _func, _is_output=False):

        self._set_type(_func)

        self._is_parents_set = False
        self._is_children_set = False
        self._is_group_set = False

        self._func = _func
        self._parents = []
        self._children = []

        self._size = self.compute_size()

        self._group = None

        self._is_output = _is_output
        self._is_liveout = True

        self._level_no = 0
        self._group_level_no = 0

        # storage info
        self._orig_storage_class = None
        self._storage_class = None
        self._array = None
        self._scratch_info = []
        self._is_pointwise = None
        
    def clone (self, inline=False):
        if not inline:
            comp = ComputeObject (self._func, self._is_output)
        else:
            comp = ComputeObject (self._func.clone (), 
                                  self._is_output)
        comp._parents = list(self.parents)
        comp._is_parents_set = self._is_parents_set
        comp._is_children_set = self._is_children_set
        comp._is_group_set = self._is_group_set
        comp._children = list(self.children)
        self._orig_storage_class = comp._orig_storage_class
        self._storage_class = comp._storage_class
        self._array = comp._array
        self._scratch_info = comp._scratch_info
        return comp
        
    def __str__ (self):
        return self._func.name
    @property
    def func(self):
        return self._func
    @property
    def is_parents_set(self):
        return self._is_parents_set
    @property
    def is_children_set(self):
        return self._is_children_set
    @property
    def is_group_set(self):
        return self._is_group_set
    @property
    def compute_type(self):
        return self._compute_type

    @property
    def is_func_type(self):
        return self._compute_type == ComputeTypes.FUNCTION
    @property
    def is_image_typ(self):
        return self._compute_type == ComputeTypes.IMAGE
    @property
    def is_reduction_typ(self):
        return self._compute_type == ComputeTypes.REDUCTION
    @property
    def is_tstencil_type(self):
        return self._compute_type == ComputeTypes.TSTENCIL

    @property
    def parents(self):
        assert self.is_parents_set
        return self._parents
    @property
    def children(self):
        assert self.is_children_set
        return self._children
    @property
    def size(self):
        return self._size
    @property
    def group(self):
        assert self.is_group_set
        return self._group
    @property
    def level(self):
        return self._level_no
    @property
    def group_level(self):
        return self._group_level_no
    @property
    def is_output(self):
        return self._is_output
    @property
    def is_liveout(self):
        return self._is_liveout
    @property
    def orig_storage_class(self):
        return self._orig_storage_class
    @property
    def storage_class(self):
        return self._storage_class
    @property
    def array(self):
        return self._array
    @property
    def scratch(self):
        return self._scratch_info
    
    def print_parent (self):
        LOG (logging.INFO, self.func.name + " <- ")
        for c in self.parents:
            LOG (logging.INFO, str(c)+ " id: " + hex(id(c)), end = ', ')
        LOG (logging.INFO, "")
    
    def print_children (self):
        raise(Exception)
        LOG (logging.INFO, self.func.name +  " -> ")
        for c in self.children:
            LOG (logging.INFO, str(c) + " id: " + hex(id(c)), end=', ')
        LOG (logging.INFO, "")
    def _set_type(self, _func):
        self._compute_type = None
        # NOTE: do NOT change this to if-elif-elif...else
        # since this is an _inheritance hierarchy. Somthing can be a
        # Function AND a reduction.
        if isinstance(_func, Function):
            self._compute_type = ComputeTypes.FUNCTION
        if isinstance(_func, Image):
            self._compute_type = ComputeTypes.IMAGE
        if isinstance(_func, Reduction):
            self._compute_type = ComputeTypes.REDUCTION
        if isinstance(_func, TStencil):
            self._compute_type = ComputeTypes.TSTENCIL

        if self._compute_type is None:
            raise TypeError("unknown compute object function type:\n"
                            "Given: %s\n"
                            "nType: %s" % (self._func, type(self.function)))
        return

    def add_child(self, comp):
        assert isinstance(comp, ComputeObject)
        self._children.append(comp)
        self._children = list(set(self._children))
        self._is_children_set = True
        return

    def add_parent(self, comp):
        assert isinstance(comp, ComputeObject)
        self._parents.append(comp)
        self._parents = list(set(self._parents))
        self._is_parents_set = True
        return

    def remove_child(self, comp):
        if comp in self._children:
            self._children.remove(comp)
        return

    def remove_parent(self, comp):
        if comp in self._parents:
            self._parents.remove(comp)
        return

    def set_parents(self, parents):
        # empty list of parents => root level comp
        if not parents:
            self._is_parents_set = True
            return
        for p in parents:
            assert isinstance(p, ComputeObject)
        self._parents = parents
        self._is_parents_set = True
        return

    def set_children(self, children):
        # empty list of children => leaf level comp
        if not children:
            self._is_children_set = True
            return
        for p in children:
            assert isinstance(p, ComputeObject)
        self._children = children
        self._is_children_set = True
        return

    def set_group(self, group):
        assert isinstance(group, Group)
        self._group = group
        self._is_group_set = True
        return

    def unset_group(self):
        self._group = None
        return

    # within the group
    def compute_liveness(self):
        assert self.is_group_set

        if self.is_output:
            self._is_liveout = True
            return

        # if there any children
        if not self.children:
            # no child => live_out
            self._is_liveout = True
            return

        self._is_liveout = False
        for child in self.children:
            # if any child is in another group
            if child.group != self.group:
                self._is_liveout = True
                break

        return
    
    def compute_size(self, sizes=None):
        '''
        For each dimension of the compute object, find the interval size and
        the Parameter associated with the dimension
        '''
        # list 'interval_sizes' : [ interval_size[dim] for dim in (0..ndims) ]
        # tuple 'interval_size' : (param, size_expr)
        interval_sizes = []
        intervals = self.func.domain
        dims = self.func.ndims

        def compute_size_tuple(dim, intervals, sizes, funcname):
            if sizes and sizes[dim] != -1:
                param = 0  # const
                size = sizes[dim]
            else:
                params = intervals[dim].collect(Parameter)
                assert not len(params) > 1, funcname+", \
                    ("+str(dim)+"/"+str(len(params))+'),'+', \
                    '.join([par.name for par in params])
                if len(params) == 1:
                    param = params[0]
                elif len(params) == 0:  # const
                    param = 0
                size = intervals[dim].upperBound - \
                       intervals[dim].lowerBound + 1
            size = simplify_expr(size)
            
            return (param, size)

        # if sizes are given, ensure it contains sizes of all dims
        if sizes:
            assert len(sizes) == dims
        
        # for each dimension
        for dim in range(0, dims):
            dim_size_tuple = \
                compute_size_tuple(dim, intervals, sizes, self._func.name)
            interval_sizes.append(dim_size_tuple)

        return interval_sizes

    def set_level(self, _level_no):
        self._level_no = _level_no
    def set_grp_level(self, _level_no):
        self._group_level_no = _level_no

    def set_orig_storage_class(self, _storage_class):
        assert isinstance(_storage_class, Storage)
        self._orig_storage_class = _storage_class
    def set_storage_class(self, _storage_class):
        assert isinstance(_storage_class, Storage)
        self._storage_class = _storage_class

    def set_storage_object(self, _array):
        if self.is_tstencil_type:
            assert isinstance(_array, tuple)
            assert len(_array) == 2
            assert isinstance(_array[0], genc.CArray)
            assert isinstance(_array[1], genc.CArray)
            self._array = _array
        else:
            assert isinstance(_array, genc.CArray)
        self._array = _array
        
    def set_scratch_info(self, _scratch_info):
        self._scratch_info = _scratch_info

    def is_pointwise (self):
        if (self._is_pointwise != None):
            return self._is_pointwise
        
        self._is_pointwise = False
        name = self.func.name
        refs = self._func.getObjects (Reference)
        
        if (not self.equal_dim_size_with_children ()):
            return False
        #A pointwise function works on only one point (x, y)
        #Hence, all references in the function body should have 
        #same index (x + k, y + k), where k is a constant
     
        if (len(refs) == 0):
            return False
            
        arg_0 = refs[0].arguments
        
        for ref in refs:
            if (len(arg_0) != len(ref.arguments)):
                return False
            
            for i in range (0, len (arg_0)):
                if (str(arg_0[i]) != str(ref.arguments[i])):
                    return False
                    
        self._is_pointwise = True
        return True
    
    def equal_dim_size_with_children (self):
        
        self_sizes = self.compute_size ()
        for k in self.children:
            child_sizes = k.compute_size ()
            
            if (len(self_sizes) != len(child_sizes)):
                return False
            for i in range(0, len(child_sizes)):
                if (str (self_sizes[i][0]) != str(child_sizes[i][0]) or
                    str (self_sizes[i][1]) != str(child_sizes[i][1])):
                    return False
        
        return True
        
    def n_refs (self):
        return len (self.func.getObjects (Reference))
        
class Group:
    """ 
        Group is a part of the pipeline which realizes a set of computation
        objects. Scheduling and storage allocation is done at the level of a 
        group. A group also maintains a polyhedral representation of the 
        computation objects when possible.
    """
    # Construct a group from a set of language functions / reductions
    def __init__(self, _ctx, _comp_objs, \
                 _param_constraints):

        self._id = IdGen.get_grp_id()

        log_level = logging.DEBUG

        # All the computation constructs in the language derive from the
        # Function class. Input images cannot be part of a group.
        self._is_image_typ = False
        for comp in _comp_objs:
            assert(isinstance(comp, ComputeObject))
            if comp.is_image_typ:
                self._is_image_typ = True

        self._comps  = _comp_objs
        self._parents = []
        self._children = []

        self._set_type()
        self.set_comp_group()

        self._level_order_comps = self.order_compute_objs()
        self._comps = self.get_sorted_comps()
        self._inputs = self.find_root_comps()
        self._live_outs = None
        self._image_refs = self.collect_image_refs()

        self._children_map = None
        self._polyrep = None
        self._tile_sizes = {}
        
        # Create a polyhedral representation if possible.
        # Currently doing extraction only when all the compute_objs
        # domains are affine. This can be revisited later.
        if self.isPolyhedral():
            self._polyrep = PolyRep(_ctx, self, [], _param_constraints)

        self._comps_schedule = None
        self._liveness_map = None
        self._total_used_size = -1
        self._param_constraints = _param_constraints
        self._ctx = _ctx
        self._inline_comps = []
        self._inline_in_all_children = None
        self._threads = -1
        
    @property
    def threads(self):
        return self._threads
    @property
    def tile_sizes(self):
        return self._tile_sizes
    @property
    def id_(self):
        return self._id
    @property
    def comps(self):
        return self._comps
    @property
    def parents(self):
        return self._parents
    @property
    def children(self):
        return self._children

    # NOTE: Current assumptions:
    # (1). A Reduction group, TStencil group, and Image group has only one
    # compute object of the respective kind.
    # (2). If a group is of pure function type, all its compute objects are
    # of Function type, and not one of the types listed in (1).
    @property
    def is_func_type(self):
        return self._group_type == ComputeTypes.FUNCTION
    @property
    def is_image_typ(self):
        return self._group_type == ComputeTypes.IMAGE
    @property
    def is_reduction_typ(self):
        return self._group_type == ComputeTypes.REDUCTION
    @property
    def is_tstencil_type(self):
        return self._group_type == ComputeTypes.TSTENCIL

    @property
    def polyRep(self):
        return self._polyrep
    @property
    def inputs(self):
        return self._inputs
    @property
    def liveouts(self):
        return self._live_outs
    @property
    def image_refs(self):
        return self._image_refs
    @property
    def name(self):
        return str([comp.func.name for comp in self.comps])

    @property
    def children_map(self):
        return self._children_map

    @property
    def get_ordered_comps(self):  # <- cant have such a name for property
        return self._level_order_comps
    @property
    def root_comps(self):
        return self._inputs

    @property
    def comps_schedule(self):
        return self._comps_schedule
    @property
    def liveness_map(self):
        return self._liveness_map
    
    @property
    def inlined_comps(self):
        return self._inline_comps
    
    def inline_in_all_children (self):
        
        if (self._inline_in_all_children != None):
            return self._inline_in_all_children
        
        self._inline_in_all_children = False
        
        if (self.is_pointwise ()):
            if (len(self.comps[0].func.getObjects(Reference)) == 1):
                self._inline_in_all_children = True
                return True
        
        #Find all top-level case statements
        case_stmts = []
        for b in self.comps[0].func.defn:
            if isinstance (b, constructs.Case):
                case_stmts += [b]
        
        if (case_stmts == []):
            return False
        
        #Determine all condition variables of case, their constant values, and
        #their position in the arguments of the function declaration
        info = [] #Tuple of [Variable, Index in Function's Declaration, Value]
        for case_stmt in case_stmts:
            cond = case_stmt.condition
            var = None
            val = None
            
            if (isinstance (cond.rhs, Value) and isinstance (cond.lhs, Variable)):
                var = cond.lhs
                val = cond.rhs
            elif isinstance (cond.lhs, Value) and isinstance (cond.rhs, Variable):
                var = cond.rhs
                val = cond.lhs
            
            try:
                lhs_index = self.comps[0].func.variables.index (var)
                if (len(info) > 0):
                    idx = len(info)-1
                    if (info[idx][0] == var and info[idx][1] == lhs_index):
                        pass
                    else:
                        return False
                info += [(var, lhs_index, val)]
            except ValueError:
                pass
        
        #Collect all references of group's func from its children
        child_to_ref = {}
        for child in self.children:
            refObjects = child.comps[0].func.getObjects(Reference)
            child_to_ref [child] = []
            for ref in refObjects:
                if (ref.objectRef == self.comps[0].func):
                    child_to_ref [child] += [ref]
                
        constant_args= []
        const_arg_pos_list = []
        for child in child_to_ref:
            refs = child_to_ref[child]
            #Make sure for all refs in a child the constant arg and
            #its position are same
            constant_arg = None
            const_arg_pos = None
                
            for ref in refs:
                args = ref.arguments
                for i in range(len(args)):
                    arg = args[i]
                    if (isinstance (arg, Value)):
                        
                        if (constant_arg is None):
                            constant_arg = arg
                            const_arg_pos = i
                        else:
                            if (constant_arg != arg or const_arg_pos != i):
                                return False
                
                if (constant_arg is None):
                    return False
            
            constant_args += [constant_arg]
            const_arg_pos_list += [const_arg_pos]

            #Determine whether the constant arg is used in
            #one of the case statement 
            found = False
            for case_info in info:
                if (case_info[2] == constant_arg and
                    case_info[1] == const_arg_pos):
                    found = True
            if not found:
                return False
        
        self._inline_in_all_children = True
        return True            
    
    def set_inline_comps (self, comps):
        self._inline_comps = list(comps)
    
    def is_pointwise (self):
        assert (len(self.comps) == 1)
        return self.comps[0].is_pointwise ()
    def recompute_computation_objs(self):
        '''After removing computations, recompute the internal representations
        '''
        self._set_type()
        self.set_comp_group()

        self._level_order_comps = self.order_compute_objs()
        self._comps = self.get_sorted_comps()
        self._inputs = self.find_root_comps()
        self._image_refs = self.collect_image_refs()

        if self.isPolyhedral():
            self._polyrep = PolyRep(self._ctx, self, [], self._param_constraints)

    def set_tile_size_for_dim(self, size, dim):
        self._tile_sizes [dim] = size
        
    def _set_type(self):
        '''
        Depends on how specifically the ComputeObject's type was set, since all
        types inherit from Function type.
        '''
        group_type = ComputeTypes.FUNCTION
        for comp in self._comps:
            group_type = comp.compute_type
            if not group_type == ComputeTypes.FUNCTION:
            # Ideally this case won't occur when the list of group's comps has
            # more than one compute object, since as of now, we group only a
            # bunch of pure compute objects. Images need no fusion since they
            # are program inputs. Fusion and/or tiling optimizations for
            # Reductions is not yet covered. TStencils will be standalone
            # groups if diamond tiling is enabled.
            # TODO: add flag to decide if TStencils are to be Diamond tiled or
            # Overlap tiled.
                break
        self._group_type = group_type
        return

    def set_comp_group(self):
        for comp in self.comps:
            comp.set_group(self)
        return

    def find_and_set_parents(self):
        parents = []
        for comp in self.comps:            
            comp_parent_groups = [p_comp.group for p_comp in comp.parents]
            parents.extend(comp_parent_groups)
        parents = list(set(parents))
        if self in parents:
            parents.remove(self)
        self.set_parents(parents)
        return parents
    def find_and_set_children(self):
        children = []
        for comp in self.comps:
            comp_children_groups = [c_comp.group for c_comp in comp.children]
            children.extend(comp_children_groups)
        children = list(set(children))
        if self in children:
            children.remove(self)
        self.set_children(children)
        return children

    def add_child(self, group):
        assert isinstance(group, Group)
        self._children.append(group)
        self._children = list(set(self._children))
        return
    def add_parent(self, group):
        assert isinstance(group, Group)
        self._parents.append(group)
        self._parents = list(set(self._parents))
        return

    def remove_child(self, group):
        if group in self._children:
            self._children.remove(group)
        return
    def remove_parent(self, group):
        if group in self._parents:
            self._parents.remove(group)
        return

    def set_parents(self, parents):
        for group in parents:
            assert isinstance(group, Group)
        self._parents = parents
        return
    def set_children(self, children):
        for group in children:
            assert isinstance(group, Group)
        self._children = children
        return

    def compute_liveness(self):
        liveouts = []
        for comp in self.comps:
            comp.compute_liveness()
            parts = self.polyRep.poly_parts[comp]
            for part in parts:
                part.compute_liveness()
            if comp.is_liveout:
                liveouts.append(comp)

        self._live_outs = liveouts
        return

    def is_fused(self):
        return len(self.comps) > 1

    def getParameters(self):
        params = []
        for comp in self.comps:
            params = params + comp.func.getObjects(Parameter)
        return list(set(params))

    def isPolyhedral(self):
        polyhedral = True
        for comp in self.comps:
            if (not comp.func.hasBoundedIntegerDomain()):
                polyhedral = False
                print("no bounded integer domain for: "+comp.func.name)
        return polyhedral

    def order_compute_objs(self):
        parents = {}
        for comp in self.comps:
            parents[comp] = comp.parents
        order = level_order(self.comps, parents)
        for comp in order:
            comp.set_grp_level(order[comp])
        return order

    def find_root_comps(self):
        root_comps = [comp for comp in self.comps \
                             if self._level_order_comps[comp] == 0]
        return root_comps

    def collect_image_refs(self):
        refs = []
        for comp in self.comps:
            refs += comp.func.getObjects(Reference)
        image_refs = [ref.objectRef for ref in refs \
                                      if isinstance(ref.objectRef, Image)]
        image_refs = list(set(image_refs))
        return image_refs

    def collect_comps_children(self):
        children_map = {}
        for comp in self.comps:
            comp_children = \
                [child for child in comp.children \
                         if child.group == self]
            if comp_children:
                children_map[comp] = comp_children
        self._children_map = children_map
        return

    def get_sorted_comps(self):
        sorted_comps = sorted(self._level_order_comps.items(),
                              key=lambda x: x[1])
        sorted_comps = [c[0] for c in sorted_comps]
        return sorted_comps

    def set_comp_and_parts_sched(self):
        self._comps_schedule = schedule_within_group(self)
        return

    def set_liveness_map(self, _liveness_map):
        self._liveness_map = _liveness_map
        return

    def get_total_size(self, param_estimates):
        size = 0
        visitor = CountMemRefsExpVisitor ()
        get_size_visitor = GetSizeVisitor (param_estimates)
        
        for comp in self.comps:
            for ast_node in comp.func.defn:
                
                expr = None
                
                sizes = comp.compute_size ()
                dim_size = 1
                for _size in sizes:
                    dim_size = dim_size*_size[1].visit (get_size_visitor)
                    
                if (isinstance (ast_node, Case)):
                    expr = ast_node.expression
                elif (isinstance (ast_node, AbstractExpression)):
                    expr = ast_node
                elif (isinstance (ast_node, Reduce)):
                    expr = ast_node.expression
                    if (isinstance (expr, Reduce)):
                        expr = expr.expression
                
                size += dim_size*expr.visit (visitor)
        
        return size
    
    def get_max_live_size(self, param_estimates):
        '''Returns maximum live data size for the group.
        The return value is in terms of number of image elements.
        The return value has to be multiplied with the size of each 
        image element.
        '''
       
        max_size = 0
        visitor = CountMemRefsExpVisitor ()
        get_size_visitor = GetSizeVisitor (param_estimates)
        
        for comp in self.comps:
            size = 0
            sizes = comp.compute_size ()
            dim_size = 1
            for _size in sizes:
                dim_size = dim_size*_size[1].visit (get_size_visitor)
            
            
            size = dim_size*len(comp.parents)
            max_size = max (max_size, size)
        
        
        return max_size
        
    def get_dimensional_reuse2 (self, param_estimates):
        class Interval (object):
            def __init__ (self, lb, ub):
                self.lb = lb
                self.ub = ub
                
        source_comps = []
        for comp in self.comps:
            parent_in = False
            for _parent in comp.parents:
                if _parent in self.comps:
                    parent_in = True
                    break
            
            if (not parent_in):
                source_comps.append (comp)
        
        get_size_visitor = GetSizeVisitor (param_estimates)
        
        def dimensional_reuse (dim, interval1, interval2, 
                               param_estimates, get_size_visitor):
            if (dim >= len (interval1) or dim >= len (interval2)):
                return None
            lb1 = interval1[dim].lowerBound.visit (get_size_visitor)
            lb2 = interval2[dim].lowerBound.visit (get_size_visitor)
            ub1 = interval1[dim].upperBound.visit (get_size_visitor)
            ub2 = interval2[dim].upperBound.visit (get_size_visitor)
            
            if (lb1 <= ub2 and ub1 <= lb2):
                return Interval (ub1, lb2)
            
            return None
        
        max_dim = 0
        for comp in self.comps:
            max_dim = max (max_dim, comp.func.ndims)
        
        dim_interval = [-1 for i in range (0, max_dim)]
        
        for _dim in range (0, max_dim):
            stack = list (source_comps)
            
            while stack.size() != 0:
                
                s = stack.pop ()
                
                for child in s.children ():
                    pass
    
    def get_size_for_each_dim (self, param_estimates):
        max_dim = 0
        for comp in self.comps:
            max_dim = max (max_dim, comp.func.ndims)
            
        dim_sizes = [0 for i in range (0, max_dim)]
        
        get_size_visitor = GetSizeVisitor (param_estimates)
        
        for comp in self.comps:
            intervals = comp.func.domain
           
            for dim in range (comp.func.ndims):
                lb = intervals[dim].lowerBound
                
                lb = lb.visit (get_size_visitor)
               
                ub = intervals[dim].upperBound
              
                ub = ub.visit (get_size_visitor)
                
                size = ub - lb + 1             
                
                dim_sizes [dim] = size

        return dim_sizes
        
    def get_dimensional_reuse (self, param_estimates, func_map):
        '''Returns the dimensional reuse in each dimension. 
        Reurns a list of dimensional reuse for each dimension.
        Dimensional Reuse is in terms of number of 4 Bytes.
        optgrouping multiplies the dimensional reuse with 4 Bytes.
        '''
        
        max_dim = 0
        
        for comp in self.comps:
            max_dim = max (max_dim, comp.func.ndims)
            
        dim_reuse = [0 for i in range (0, max_dim)]
        
        get_size_visitor = GetSizeVisitor (param_estimates)
        
        for comp in self.comps:
            intervals = comp.func.domain

            if isinstance(comp.func, Reduction):
                continue
            
            refs = comp.func.getObjects (Reference)
            funcs = []
            for ref in refs:
                funcs += [ref.objectRef]
            
            funcs = set (funcs)
            to_exclude = []
            for f in funcs:
                s = func_map[f].compute_size ()
                m = 1
                d = 0
                for a in s:
                    if (isinstance (a[1], Value)):
                        m = m*a[1].value
                        d += 1
                    else:
                        break
                if (d == f.ndims and m > 1 and m < 1000):
                    to_exclude += [func_map[f]]
                
            for dim in range (comp.func.ndims):
               
                lb = intervals[dim].lowerBound
                
                lb = lb.visit (get_size_visitor)
               
                ub = intervals[dim].upperBound
               
                ub = ub.visit (get_size_visitor)
               
                size = ub - lb + 1
                
                first_iter = lb + int((ub - lb)/2)
                second_iter = first_iter + 1
                comps_to_exclude = []
                
                first_iter_visitor = MemRefsAtIterationVisitor (dim, first_iter, to_exclude)
                second_iter_visitor = MemRefsAtIterationVisitor (dim, second_iter, to_exclude)
                mem_ref_at_second_iter = set()
                mem_ref_at_first_iter = set()
                
                for ast_node in comp.func.defn:
                    expr = None
                    if (isinstance (ast_node, Case)):
                        expr = ast_node.expression
                    elif (isinstance (ast_node, AbstractExpression)):
                        expr = ast_node
                    elif (isinstance (ast_node, Reduce)):
                        expr = ast_node.expression
                        if (isinstance (expr, Reduce)):
                            expr = expr.expression

                    expr.visit(first_iter_visitor)
                    mem_ref_at_first_iter = mem_ref_at_first_iter.union (set(first_iter_visitor.dim_refs))
                    expr.visit(second_iter_visitor)
                    mem_ref_at_second_iter = mem_ref_at_second_iter.union (set(second_iter_visitor.dim_refs))
                
                dim_reuse_iters = mem_ref_at_second_iter.intersection (mem_ref_at_first_iter)
                dim_size = 1
                dim_reuse[dim + (max_dim-comp.func.ndims)] += len (dim_reuse_iters)*dim_size
        
        return dim_reuse
    
    def is_dim_tileable (self, dim, slope_min, dim_size):
        return slope_min [dim] != '*' and dim_size[dim] != 0

    def set_total_used_size (self, total_used_size):
        self._total_used_size = total_used_size
    def set_n_buffers (self, n_buffers):
        self._n_buffers = n_buffers
    def set_type_of_access (self, param_estimates):
        '''Returns and array of type of access
        Each type is "rectangular" if accesses are more rectangular i.e.
        f(x,y) = in(x, y-2)+in(x, y-1)+in(x,y)+in(x,y+1)+in(x,y+2)
        Or "sqaurer" if accesses are more squarer i.e.
        f(x,y) = img(x-1, y-1)*(-1.0/12) + img(x-1, y+1)*(1.0/12) + \
                           img(x, y-1)*(-2.0/12) + img(x, y+1)*(2.0/12) + \
                           img(x+1, y-1)*(-1.0/12) + img(x+1, y+1)*(1.0/12)
        '''
       
        max_dim = 0
        for comp in self.comps:
            max_dim = max (max_dim, comp.func.ndims)
        
        dim_reuse = [0 for i in range (0, max_dim)]
        access_types = []
        
        get_size_visitor = GetSizeVisitor (param_estimates)
        
        for comp in self.comps:
            intervals = comp.func.domain
            
            if isinstance(comp.func, Reduction):
                continue
            
            dims_lb = []
            
            for dim in range (comp.func.ndims):
                lb = intervals[dim].lowerBound
                lb = lb.visit (get_size_visitor)
                dims_lb.append (lb+1)
            
            access_type_visitor = AccessTypesVisitor (dims_lb, param_estimates)
            mem_refs = set()
                
            for ast_node in comp.func.defn:
                expr = None
                if (isinstance (ast_node, Case)):
                    expr = ast_node.expression
                elif (isinstance (ast_node, AbstractExpression)):
                    expr = ast_node
                elif (isinstance (ast_node, Reduce)):
                    expr = ast_node.expression
                    if (isinstance (expr, Reduce)):
                        expr = expr.expression
                        
                expr.visit(access_type_visitor)
                
                #mem_refs = mem_refs.union (set(access_type_visitor._refs))
            
            references = access_type_visitor._refs
            for ref in references:
                ref_types = []
                __dim = 0
                for ref_dim in ref:
                    if (ref_dim is Variable):
                        ref_types += "variable"
                    elif (ref_dim is AbstractBinaryOpNode):
                        if (ref_dim.op == '+' or ref_dim == '-'):
                            if ((ref_dim.left is Variable or ref_dim.left is Value) and
                                (ref_dim.right is Variable or ref_dim.right is Value)):
                                pass
                                
                    else:
                        raise Exception ("Case Not Handled for ref type")
                        
            
        self._access_types = access_types
        
        return access_types

    def get_tile_sizes_for_cache_size (self, param_estimates, slope_min,
                                       slope_max, group_parts, dim_reuse, 
                                       dim_sizes, CACHE_SIZE, LAST_DIM_SIZE,
                                       cores):
        '''Returns the tile sizes for each dimension found
            and tile_size 
        '''
        tile_sizes = {}
        total_used_size = self._total_used_size/IMAGE_ELEMENT_SIZE
        tile_size = total_used_size/cores
        if (tile_size > CACHE_SIZE):
            tile_size = CACHE_SIZE
        tile_size = tile_size/self._n_buffers
            
        LOG (logging.DEBUG, "tileable dims:")
        for i in range (len(slope_min)):
            if (self.is_dim_tileable (i, slope_min, dim_sizes)):
                LOG (logging.DEBUG, str(i))

        LOG (logging.DEBUG, "tile_size " + str(tile_size))
        LOG (logging.DEBUG, "dim_reuse " + str(dim_reuse))
        LOG (logging.DEBUG, "dim_sizes " + str(dim_sizes))
        
        last_tileable_dim = None
        n_tileable_dims = 0
        max_dim_reuse = 0
        n_non_zero_dims = 0
        n_zero_dims = 0
        outer_tileable_dim = -1
        
        for i in range (len(slope_min)):
            if self.is_dim_tileable (i, slope_min, dim_sizes):
                n_tileable_dims+=1
                last_tileable_dim = i
                if (outer_tileable_dim == -1 and 
                    dim_sizes[i] < OUTER_DIM_TILING_THRESH):
                    outer_tileable_dim = i
            else:
                outer_tileable_dim = i
                        
        if (n_tileable_dims == 0):
            self._tile_sizes = {}
            for dim in range (len(slope_min)):
                tile_sizes [dim] = int(get_next_power_of_2 (int(tile_size ** (1.0/len(slope_min))))/2)
            
            tile_sizes [max(tile_sizes)] = LAST_DIM_SIZE
            
            return tile_sizes, tile_size
        
        LOG (logging.DEBUG, 'last_tileable_dim ' + str(last_tileable_dim))
        LOG (logging.DEBUG, 'outer_tileable_dim ' + str(outer_tileable_dim))
        tile_sizes [last_tileable_dim] = min (dim_sizes[last_tileable_dim], LAST_DIM_SIZE)
        remaining_tile_size = tile_size/tile_sizes [last_tileable_dim]
        
        if (outer_tileable_dim != -1 and dim_sizes[outer_tileable_dim] != 0):
            remaining_tile_size = remaining_tile_size/dim_sizes[outer_tileable_dim]

        LOG (logging.DEBUG, "remaining_tile_size " + str(remaining_tile_size))

        for i in range (len(slope_min)):
            if (self.is_dim_tileable (i, slope_min, dim_sizes) and 
                i != last_tileable_dim and i != outer_tileable_dim):
                if (max_dim_reuse < dim_reuse [i]):
                    max_dim_reuse = dim_reuse[i]
                    max_dim = i
        
        max_dim_tile_size = remaining_tile_size
        for i in range (len(slope_min)):
            if (self.is_dim_tileable (i, slope_min, dim_sizes) and 
                i != last_tileable_dim and i != outer_tileable_dim):
                if (dim_reuse [i] != 0):
                    n_non_zero_dims += 1
                    max_dim_tile_size = max_dim_tile_size * (max_dim_reuse/dim_reuse [i])
                else:
                    n_zero_dims +=1
        
        LOG (logging.DEBUG, "max_dim_tile_size " + str(max_dim_tile_size))
        
        if (n_non_zero_dims > 0):
            max_dim_tile_size = max_dim_tile_size ** (1.0/n_non_zero_dims)
                    
            for i in range (len(slope_min)):
                if (self.is_dim_tileable (i, slope_min, dim_sizes) and 
                    i != last_tileable_dim and i != outer_tileable_dim):
                    if (dim_reuse[i] != 0):
                        tile_sizes [i] = int(max_dim_tile_size * dim_reuse[i]/max_dim_reuse)
                        LOG (logging.DEBUG, "tile_sizes [i] " + str(tile_sizes [i]))
                        if (tile_sizes [i] > dim_sizes [i]):
                            tile_sizes [i] = dim_sizes [i]
        
        if (n_zero_dims > 0):
            max_dim_tile_size = max_dim_tile_size ** (1.0/n_zero_dims)
            
            for i in range (len(slope_min)):
                if (self.is_dim_tileable (i, slope_min, dim_sizes) and i != last_tileable_dim
                    and i != outer_tileable_dim and n_zero_dims > 0):
                    if (max_dim_tile_size > dim_sizes [i]):
                        max_dim_tile_size = max_dim_tile_size ** (1.0/n_zero_dims)
                        n_zero_dims -= 1
 
            for i in range (len(slope_min)):
                if (self.is_dim_tileable (i, slope_min, dim_sizes) and i != last_tileable_dim
                    and i != outer_tileable_dim):
                    tile_sizes [i] = int (max_dim_tile_size)
        
        _tile_sizes = dict(tile_sizes)
        LOG (logging.DEBUG, "tile_sizes before popping " + str(tile_sizes))
        
        for k in tile_sizes.keys():
            if (int(tile_sizes[k]) == 0):
                _tile_sizes.pop (k)
        
        tile_sizes = _tile_sizes
        LOG (logging.DEBUG, "tile_sizes after popping " + str(_tile_sizes))
        
        if (len(slope_min) == 3):
            if (len(tile_sizes) == 1):
                if 1 not in tile_sizes and 2 in tile_sizes:
                    tile_sizes [1] = int(math.ceil(tile_size/tile_sizes[2]))
            elif (len(tile_sizes) == 2 and 1 in tile_sizes and int(tile_sizes[1]) == 1):
                tile_sizes [1] = int(math.ceil(tile_size/tile_sizes[2]))
        
        for i in range (outer_tileable_dim+1, last_tileable_dim+1):
            if i not in _tile_sizes:
                return -1, -1
                
        return _tile_sizes, tile_size
            
    def get_tile_sizes (self, param_estimates, slope_min, slope_max, group_parts, 
                        h, func_map, use_only_l2 = False, multi_level_tiling = False):
        global L2_CACHE_SIZE
        LOG (logging.DEBUG, "get_tile_sizes for " + str(self))

        if (self._total_used_size == -1):
            assert (False)
                        
        tileable_dims = set()
        tile_size = 0
        dim_reuse = [i*IMAGE_ELEMENT_SIZE for i in self.get_dimensional_reuse (param_estimates, func_map)]
        LOG (logging.DEBUG, "total used size for " + str(self) + " is " + str(self._total_used_size))
        dim_sizes = {}
        #if ("blury" in str(self)):
            #multi_level_tiling = False
            
        for i in range(1, len(slope_min) + 1):
            # Check if every part in the group has enough iteration
            # points in the dimension to benefit from tiling.
            tile = False
            for part in group_parts:
                lower_bound = part.sched.range().dim_min(i)
                upper_bound = part.sched.range().dim_max(i)
                LOG (logging.DEBUG, "lower_bound " +str(lower_bound) + " upper_bound " + str(upper_bound))
                size = upper_bound.sub(lower_bound)
                if (size.is_cst() and size.n_piece() == 1):
                    aff = (size.get_pieces())[0][1]
                    val = aff.get_constant_val()
                    if (val > TILING_THRESHOLD):
                        if i-1 in dim_sizes:
                            dim_sizes[i-1] = max (val.to_python(), dim_sizes[i-1])
                        else:
                            dim_sizes[i-1] = val.to_python()
                        #if (i-1 == 3):
                            #TODO: Correct this for z dimension of blurz, blurx, blury in bilateral_grid
                            #Use compute size instead of this way
                            #dim_sizes[i-1] = 14
                    else:
                        if i-1 not in dim_sizes:
                            dim_sizes [i-1] = 0
        
        self._dim_sizes_part = dim_sizes
        overlap_shifts_zero = False
        
        if (multi_level_tiling):
            overlap_shifts_zero = True
            for i in range(len(slope_min)):
                if (slope_min[i] != '*'):
                    right = int(math.floor(Fraction(slope_min[i][0],
                                                    slope_min[i][1])))
                    left = int(math.ceil(Fraction(slope_max[i][0],
                                                slope_max[i][1])))
                    # Compute the overlap shift
                    overlap_shift = abs(left * (h)) + abs(right * (h))
                    if (overlap_shift != 0):
                        overlap_shifts_zero = False
        
        #if ((not use_only_l2 and not multi_level_tiling) 
        #    or (multi_level_tiling and not overlap_shifts_zero)):
        
        if (not multi_level_tiling):
            if (not use_only_l2):
                tile_size_less_than_l1 = False
                cores = N_CORES
                total_used_size = self._total_used_size/IMAGE_ELEMENT_SIZE
                tile_size = total_used_size/cores
                LOG (logging.DEBUG, "tile_vol for 16 cores " +str(tile_size))
                if (tile_size < L1_CACHE_SIZE):
                    tile_size_less_than_l1 = True
                    cores = int(total_used_size/L1_CACHE_SIZE)
                    if (cores <= 1):
                        cores = 2
                
                #try with L1 Cache
                tile_sizes, tile_size = self.get_tile_sizes_for_cache_size (param_estimates, 
                    slope_min, slope_max, group_parts, dim_reuse, dim_sizes, 
                    L1_CACHE_SIZE, L1_INNER_MOST_DIM_SIZE, cores)
                
                overlap_shift_greater = False
                if (tile_sizes != -1 and tile_size != -1):
                    if (tile_size_less_than_l1):
                        self._tile_sizes = tile_sizes
                        LOG (logging.DEBUG, "tile_size_less_than_l1=True tile_sizes from L1 " +str(tile_sizes) +" cores "+str(cores))
                        return tile_size
                    
                    for i in tile_sizes.keys ():
                        if (slope_min[i] != '*'):
                            right = int(math.floor(Fraction(slope_min[i][0],
                                                            slope_min[i][1])))
                            left = int(math.ceil(Fraction(slope_max[i][0],
                                                        slope_max[i][1])))
                            # Compute the overlap shift
                            overlap_shift = abs(left * (h)) + abs(right * (h))
                                
                            if (overlap_shift + 0 > tile_sizes[i]):
                                overlap_shift_greater = True
                                break
                    
                    threshold_tile_size_met = True
                    for i in tile_sizes.keys ():
                        if tile_sizes[i] <= 2:
                            threshold_tile_size_met = False
                            break
                else:
                    overlap_shift_greater = True
                    threshold_tile_size_met = False
                        
                LOG (logging.DEBUG, "tile_sizes from L1 cores " + str(tile_sizes) + " " + str(cores))

                if (not overlap_shift_greater and threshold_tile_size_met):
                    self._tile_sizes = tile_sizes
                    #if (len(self.comps) == 2 and "filtered" in str(self)):
                    #    self._tile_sizes= {1:32, 2:256}
                        
                    return tile_size

            tile_sizes, tile_size = self.get_tile_sizes_for_cache_size (param_estimates, 
                slope_min, slope_max, group_parts, dim_reuse, dim_sizes, 
                L2_CACHE_SIZE, L2_INNER_MOST_DIM_SIZE, N_CORES)
            
            LOG (logging.DEBUG, "tile_sizes from L2 " + str(tile_sizes))
            self._tile_sizes = tile_sizes
            
            if ("denoised" in str(self)):
                #TODO: denoised group requires even tile size in dimension 0
                self._tile_sizes[0] -= 1
                
            return tile_size
            
        else:
            tile_size_less_than_l1 = False
            cores = N_CORES
            total_used_size = self._total_used_size/IMAGE_ELEMENT_SIZE
            _tile_size = total_used_size/cores
            LOG (logging.DEBUG, "tile_vol for 16 cores " + str(tile_size))
            if (_tile_size < L1_CACHE_SIZE):
                tile_size_less_than_l1 = True
                cores = int(total_used_size/L1_CACHE_SIZE)
                if (cores <= 1):
                    cores = 2
            
            #try with L1 Cache
            l1tile_sizes, l1tile_size = self.get_tile_sizes_for_cache_size (param_estimates, 
                slope_min, slope_max, group_parts, dim_reuse, dim_sizes, 
                L1_CACHE_SIZE, L1_INNER_MOST_DIM_SIZE, cores)
            
            if (tile_size_less_than_l1):
                self._tile_sizes = l1tile_sizes
                LOG (logging.DEBUG, "tile_size_less_than_l1=True tile_sizes from L1 " + str(l1tile_sizes) + " cores " + str(cores))
                return l1tile_size
            
            if (l1tile_sizes == -1):
                overlap_shift_greater = False
                overlap_shifts = {}
                for i in l1tile_sizes.keys ():
                    if (slope_min[i] != '*'):
                        right = int(math.floor(Fraction(slope_min[i][0],
                                                        slope_min[i][1])))
                        left = int(math.ceil(Fraction(slope_max[i][0],
                                                    slope_max[i][1])))
                        # Compute the overlap shift
                        overlap_shift = abs(left * (h)) + abs(right * (h))
                        overlap_shifts [i] = overlap_shift    
                        if (overlap_shift + 0 > l1tile_sizes[i]):
                            overlap_shift_greater = True
                            break
            else:
                overlap_shift_greater = True
                    
            LOG (logging.DEBUG, "tile_sizes from L1 " + str(l1tile_sizes))
            
            if (not overlap_shift_greater):
                total_used_size = self._total_used_size/IMAGE_ELEMENT_SIZE
                tile_size = total_used_size/N_CORES
                last_dim = len(dim_reuse) - 1
                LOG (logging.DEBUG, "not overlap greater total_used_size " + str(total_used_size) + " tile_size " + str(tile_size))
                
                if (tile_size < 2*L2_CACHE_SIZE or last_dim not in l1tile_sizes or 
                    len([d for d in dim_reuse if d > 0]) == 0):
                    #if tile size is less than L2 cache then return the L1 tile size
                    #no need of making code more complex, such a program will 
                    #automatically have a good L2 locality.
                    #Also, no need to do multi level tiling if dimensional reuse
                    #is 0 along all the dimensions
                    tile_sizes = l1tile_sizes
                    tile_size = l1tile_size
                else:
                    #Overlap is not greater than L1, hence, increase the L2 tile sizes
                    #in each dimension according to the dim reuse.
                    tile_size = l1tile_size*(L2_CACHE_SIZE/L1_CACHE_SIZE)
                    if (last_dim in l1tile_sizes and dim_reuse [last_dim] > 0):
                        LOG (logging.DEBUG, "overlap_shifts " + str(overlap_shifts) + \
                                          " l1tile_sizes " + str(l1tile_sizes) + \
                                          " dim_sizes " + str(dim_sizes) + \
                                          " last_dim " + str(last_dim))
                        if (l1tile_sizes[last_dim]*2 + overlap_shifts[last_dim] < dim_sizes[last_dim]):
                            last_dim_size = l1tile_sizes[last_dim]*2
                        else:
                            last_dim_size = l1tile_sizes[last_dim]
                    else:
                        last_dim_size = l1tile_sizes[last_dim]
                        
                    tile_sizes, tile_size = self.get_tile_sizes_for_cache_size(param_estimates, 
                        slope_min, slope_max, group_parts, dim_reuse, dim_sizes, 
                        L2_CACHE_SIZE, last_dim_size, N_CORES)
            else:
                #overlap size is greater than L1, hence, generate an L2 tile size
                #and then generate an L1 tile size
                tile_sizes, tile_size = self.get_tile_sizes_for_cache_size (param_estimates, 
                    slope_min, slope_max, group_parts, dim_reuse, dim_sizes, 
                    L2_CACHE_SIZE, L2_INNER_MOST_DIM_SIZE, N_CORES)
                
                l1tile_sizes, l1tile_size = self.get_tile_sizes_for_cache_size (param_estimates, 
                    slope_min, slope_max, group_parts, dim_reuse, dim_sizes, 
                    L1_CACHE_SIZE, L2_INNER_MOST_DIM_SIZE, N_CORES)
                    
            if (tile_sizes != l1tile_sizes):
                tile_sizes_keys = list(tile_sizes.keys ())
                LOG (logging.DEBUG, "tile_sizes " + str(tile_sizes) + " " +\
                                    str(l1tile_sizes))
                for k in tile_sizes_keys:
                    if (0 in tile_sizes):
                        tile_sizes [0] = min (50, tile_sizes[0])
                    elif (1 in tile_sizes):
                        tile_sizes [1] = min (64, tile_sizes[1])
                    if (k in l1tile_sizes and tile_sizes[k] >= l1tile_sizes[k]):
                        if ((l1tile_sizes[k] == 16 or l1tile_sizes[k] == 256) and 
                            "filtered" in str(self)): 
                            #Bug in Bilateral Grid group [interpolate, filtered].
                            #Seg Fault due to 16, and 8.
                            #Runs fine if l1 tile size is not multiple of l2 tile size.
                            #In Multi Level Tiling, if tile_sizes[1] is 64, then Bilateral Grid
                            #runs very fast. Also, l1 tile size [1] should be set to 
                            #10, not 16. Correct the tile size determination algorithm
                            l1tile_sizes[k] -= 1
                        if ("denoised" in str(self)):
                            if (l1tile_sizes[k] %2 == 1):
                                l1tile_sizes[k] -= 1
                            if (tile_sizes[k] %2 == 1):
                                tile_sizes[k] -= 1
                        
                        tile_sizes["L1"+str(k)] = l1tile_sizes[k]
                
                if ("interpolated" in str(self) and "filtered" in str(self)):
                    #TODO: Correct this
                    tile_sizes[1] = 42*4
                if ("blury" in str(self) and "blurx" in str(self) and "blurz" in str(self)):
                    tile_sizes[1] = 19*2
                    tile_sizes[2] = 25*2
            #if (len (self.comps) == 4):
                #if (1 in l1tile_sizes):
                #    For Bilateral Grid
                #    tile_sizes = dict()
                #    tile_sizes [1] = 64 #l1tile_sizes[1]
                #    tile_sizes ['L11'] = 15
                #    tile_sizes ["L12"] = 257
                #    tile_sizes [2] = 512
                
                #For Unsharp Mask
                #tile_sizes = dict ()
                #tile_sizes[1] = 42
                #tile_sizes[2] = 512
                #tile_sizes["L11"] = l1tile_sizes[1]
                #tile_sizes["L12"] = l1tile_sizes[2]
                
            LOG (logging.DEBUG, "tile_sizes from L2 " + str(tile_sizes))
            self._tile_sizes = tile_sizes
            return tile_size
    
    def __str__(self):
        comp_str  = '[' + \
                    ', '.join([comp.func.name \
                        for comp in self.comps]) + \
                    ']'
        return comp_str


class Pipeline:
    class MachineInformation:
        @staticmethod
        def get_dpfusion_weights (machine, is_inlining, is_heirarichal_tiling,
                                  is_multi_level_tiling):
            to_ret = {}
            if (machine == 'polymage'):
                if is_inlining and is_heirarichal_tiling and is_multi_level_tiling:
                    to_ret["DIM_STD_DEV_WEIGHT"] = 1.5;
                    to_ret["LIVE_SIZE_TO_TILE_SIZE_WEIGHT"] = 1.0;
                    to_ret["CLEANUP_THREADS_WEIGHT"] = 100.0;
                    to_ret["RELATIVE_OVERLAP_WEIGHT"] = 1000.0*50.0*1.5;
                    
                elif is_inlining and is_heirarichal_tiling:
                    to_ret["DIM_STD_DEV_WEIGHT"] = 1.5;
                    to_ret["LIVE_SIZE_TO_TILE_SIZE_WEIGHT"] = 1.0;
                    to_ret["CLEANUP_THREADS_WEIGHT"] = 100.0;
                    to_ret["RELATIVE_OVERLAP_WEIGHT"] = 1000.0*50.0*1.5;
                    
                elif is_inlining and is_multi_level_tiling:
                    to_ret["DIM_STD_DEV_WEIGHT"] = 1.5;
                    to_ret["LIVE_SIZE_TO_TILE_SIZE_WEIGHT"] = 1.5;
                    to_ret["CLEANUP_THREADS_WEIGHT"] = 100.0;
                    to_ret["RELATIVE_OVERLAP_WEIGHT"] = 1000.0*50.0/1.5;
                
                elif is_multi_level_tiling and is_heirarichal_tiling:
                    to_ret["DIM_STD_DEV_WEIGHT"] = 1.5;
                    to_ret["LIVE_SIZE_TO_TILE_SIZE_WEIGHT"] = 1.0;
                    to_ret["CLEANUP_THREADS_WEIGHT"] = 100.0;
                    to_ret["RELATIVE_OVERLAP_WEIGHT"] = 1000.0*50.0*1.5;
                    
                elif is_inlining:
                    to_ret["DIM_STD_DEV_WEIGHT"] = 1.5;
                    to_ret["LIVE_SIZE_TO_TILE_SIZE_WEIGHT"] = 1.0;
                    to_ret["CLEANUP_THREADS_WEIGHT"] = 100.0;
                    to_ret["RELATIVE_OVERLAP_WEIGHT"] = 1000.0*50.0*1.5;
                    
                elif is_heirarichal_tiling:
                    to_ret["DIM_STD_DEV_WEIGHT"] = 1.5;
                    to_ret["LIVE_SIZE_TO_TILE_SIZE_WEIGHT"] = 1.0;
                    to_ret["CLEANUP_THREADS_WEIGHT"] = 100.0;
                    to_ret["RELATIVE_OVERLAP_WEIGHT"] = 1000.0*50.0*1.5;
                    
                elif is_multi_level_tiling:
                    to_ret["DIM_STD_DEV_WEIGHT"] = 1.5;
                    to_ret["LIVE_SIZE_TO_TILE_SIZE_WEIGHT"] = 1.5;
                    to_ret["CLEANUP_THREADS_WEIGHT"] = 100.0;
                    to_ret["RELATIVE_OVERLAP_WEIGHT"] = 1000.0*50.0/1.5;
                    
                else:
                    to_ret["DIM_STD_DEV_WEIGHT"] = 1.5;
                    to_ret["LIVE_SIZE_TO_TILE_SIZE_WEIGHT"] = 1.0;
                    to_ret["CLEANUP_THREADS_WEIGHT"] =100.0;
                    to_ret["RELATIVE_OVERLAP_WEIGHT"] = 1000.0*50.0*1.5;
            
            elif (machine == 'mcastle2'):
                if (is_multi_level_tiling):
                    global L2_CACHE_SIZE
                    #In case of multi level tiling L2_CACHE_SIZE is set to 256KB
                    #the weights are also for that cache size
                    L2_CACHE_SIZE = int(256*1024/IMAGE_ELEMENT_SIZE)
                if is_inlining and is_heirarichal_tiling and is_multi_level_tiling:
                    to_ret["DIM_STD_DEV_WEIGHT"] = 1.5
                    to_ret["LIVE_SIZE_TO_TILE_SIZE_WEIGHT"] = 1.0
                    to_ret["CLEANUP_THREADS_WEIGHT"] = 100.0
                    to_ret["RELATIVE_OVERLAP_WEIGHT"] = 1000.0*50.0*1.5
                    
                elif is_inlining and is_heirarichal_tiling:
                    to_ret["DIM_STD_DEV_WEIGHT"] = 1.5
                    to_ret["LIVE_SIZE_TO_TILE_SIZE_WEIGHT"] = 1.0
                    to_ret["CLEANUP_THREADS_WEIGHT"] = 100.0
                    to_ret["RELATIVE_OVERLAP_WEIGHT"] = 1000.0*50.0*1.5
                    
                elif is_inlining and is_multi_level_tiling:
                    to_ret["DIM_STD_DEV_WEIGHT"] = 2.0
                    to_ret["LIVE_SIZE_TO_TILE_SIZE_WEIGHT"] = 1.0
                    to_ret["CLEANUP_THREADS_WEIGHT"] = 100.0
                    to_ret["RELATIVE_OVERLAP_WEIGHT"] = 1000.0*50.0/1.2
                
                elif is_multi_level_tiling and is_heirarichal_tiling:
                    to_ret["DIM_STD_DEV_WEIGHT"] = 1.5
                    to_ret["LIVE_SIZE_TO_TILE_SIZE_WEIGHT"] = 1.0
                    to_ret["CLEANUP_THREADS_WEIGHT"] = 100.0
                    to_ret["RELATIVE_OVERLAP_WEIGHT"] = 1000.0*50.0*1.5
                    
                elif is_inlining:
                    to_ret["DIM_STD_DEV_WEIGHT"] = 1.5
                    to_ret["LIVE_SIZE_TO_TILE_SIZE_WEIGHT"] = 1.0
                    to_ret["CLEANUP_THREADS_WEIGHT"] = 100.0
                    to_ret["RELATIVE_OVERLAP_WEIGHT"] = 1000.0*50.0*1.5
                    
                elif is_heirarichal_tiling:
                    to_ret["DIM_STD_DEV_WEIGHT"] = 1.5
                    to_ret["LIVE_SIZE_TO_TILE_SIZE_WEIGHT"] = 1.0
                    to_ret["CLEANUP_THREADS_WEIGHT"] = 100.0
                    to_ret["RELATIVE_OVERLAP_WEIGHT"] = 1000.0*50.0*1.5
                    
                elif is_multi_level_tiling:
                    to_ret["DIM_STD_DEV_WEIGHT"] = 2.0
                    to_ret["LIVE_SIZE_TO_TILE_SIZE_WEIGHT"] = 1.0
                    to_ret["CLEANUP_THREADS_WEIGHT"] = 100.0
                    to_ret["RELATIVE_OVERLAP_WEIGHT"] = 1000.0*50.0/1.2
                    
                else:
                    to_ret["DIM_STD_DEV_WEIGHT"] = 2.0
                    to_ret["LIVE_SIZE_TO_TILE_SIZE_WEIGHT"] = 0.3
                    to_ret["CLEANUP_THREADS_WEIGHT"] = 100.0
                    to_ret["RELATIVE_OVERLAP_WEIGHT"] = 1000.0*50.0*1.5
            else:
                raise Exception ("Unknown Machine. Enter Weights for the new machine")
                
            return to_ret
        @staticmethod
        def get_dim_std_dev_weight (is_inlining, is_heirarichal_tiling,
                                    is_multi_level_tiling):
            return Pipeline.MachineInformation.get_dpfusion_weights (MACHINE_TYPE, 
                                                            is_inlining, 
                                                            is_heirarichal_tiling,
                                                            is_multi_level_tiling) ["DIM_STD_DEV_WEIGHT"]
        @staticmethod
        def get_live_to_tile_size_weight (is_inlining, is_heirarichal_tiling,
                                          is_multi_level_tiling):
            return Pipeline.MachineInformation.get_dpfusion_weights (MACHINE_TYPE, 
                                                            is_inlining,
                                                            is_heirarichal_tiling,
                                                            is_multi_level_tiling) ["LIVE_SIZE_TO_TILE_SIZE_WEIGHT"]
        @staticmethod
        def get_cleanup_threads_weight (is_inlining, is_heirarichal_tiling,
                                        is_multi_level_tiling):
            return Pipeline.MachineInformation.get_dpfusion_weights (MACHINE_TYPE, 
                                                            is_inlining, 
                                                            is_heirarichal_tiling,
                                                            is_multi_level_tiling) ["CLEANUP_THREADS_WEIGHT"]
        @staticmethod                                 
        def get_relative_overlap_weight (is_inlining, is_heirarichal_tiling,
                                         is_multi_level_tiling):
            return Pipeline.MachineInformation.get_dpfusion_weights (MACHINE_TYPE, 
                                                            is_inlining, is_heirarichal_tiling,
                                                            is_multi_level_tiling) ["RELATIVE_OVERLAP_WEIGHT"]
        @staticmethod
        def get_machine_l1_cache_size ():
            return L1_CACHE_SIZE
        @staticmethod
        def get_machine_l2_cache_size ():
            return L2_CACHE_SIZE
        @staticmethod
        def get_machine_ncores ():
            return N_CORES
        @staticmethod    
        def get_machine_image_element_size ():
            return IMAGE_ELEMENT_SIZE
            
    def __init__(self, _ctx, _outputs,
                 _param_estimates, _param_constraints,
                 _grouping, _group_size, _inline_directives,
                 _tile_sizes, _size_threshold,
                 _options, _name = None):
        # Name of the pipleline is a concatenation of the names of the 
        # pipeline outputs, unless it is explicitly named.
        if _name is None:
            _name = ''.join([out.name for out in _outputs])

        self._name = _name

        self._ctx = _ctx
        self._orig_outputs = _outputs
        self._param_estimates = _param_estimates
        self._param_constraints = _param_constraints
        self._grouping = _grouping
        self._group_size = _group_size
        self._inline_directives = _inline_directives
        self._options = _options
        self._size_threshold = _size_threshold
        self._tile_sizes = _tile_sizes
        self._dim_reuse = {}
        self._do_inline = 'inline' in self._options

        ''' CONSTRUCT DAG '''
        # Maps from a compute object to its parents and children by
        # backtracing starting from given live-out functions.
        # TODO: see if there is a cyclic dependency between the compute
        # objects. Self references are not treated as cycles.
        self._orig_funcs = get_funcs(self._orig_outputs)

        self._inputs = []

        ''' CLONING '''
        # Clone the functions and reductions
        self._clone_map = {}
        for func in self._orig_funcs:
            if isinstance(func, Image):
                self._clone_map[func] = func
                self._inputs.append(func)
            else:
                self._clone_map[func] = func.clone()
        self._outputs = [self._clone_map[obj] for obj in self._orig_outputs]
        # Modify the references in the cloned objects (which refer to
        # the original objects)
        for func in self._orig_funcs:
            cln = self._clone_map[func]
            refs = cln.getObjects(Reference)
            for ref in refs:
                if not isinstance(ref.objectRef, Image):
                    ref._replace_ref_object(self._clone_map[ref.objectRef])

        ''' DAG OF CLONES '''
        self._func_map, self._comps = \
            self.create_compute_objects()

        self._level_order_comps = self.order_compute_objs()
        
        self._comps = self.get_sorted_comps()

        ''' INITIAL GROUPING '''
        # Create a group for each pipeline function / reduction and compute
        # maps for parent and child relations between the groups
        self._groups = self.build_initial_groups()
        # Store the initial pipeline graph. The compiler can modify 
        # the pipeline by inlining functions.
        # self._initial_graph = self.draw_pipeline_graph()

        # Checking bounds
        bounds_check_pass(self)
        #self._inline_directives = [self._groups[1]]
        # inline pass
        #TODO: Remove
        inline_after_grouping = True
        if (self._do_inline):
            self.pre_grouping_inline_phase ()
            
        if (not inline_after_grouping):
            inline_pass(self)
            
        # make sure the set of functions to be inlined and those to be grouped
        # are disjoint
        if self._inline_directives and self._grouping:
            group_comps = []
            for g in self._grouping:
                group_funcs += g
            a = set(self._inline_directives)
            b = set(group_funcs)
            assert a.isdisjoint(b)

        #self.dim_reuse_for_each_group ()
        ''' GROUPING '''
        # TODO check grouping validity
        if self._grouping and False:
            # for each group
            for g in self._grouping:
                # get clones of all functions
                clones = [self._clone_map[f] for f in g]
                comps = [self.func_map[f] for f in clones]
                # list of group objects to be grouped
                merge_group_list = \
                    [comp.group for comp in comps]
                if len(merge_group_list) > 1:
                     merged = merge_group_list[0]
                     for i in range(1, len(merge_group_list)):
                        merged = self.merge_groups(merged, merge_group_list[i])
        else:
            # Run the grouping algorithm
            #TESTING HERE
            self.initialize_storage()
            if 'dpfusion' in self.options:
                auto_group_dp(self)
            else:
                auto_group_greedy(self)

            #auto_group(self)
            #self.merge_groups (self.groups[1], self.groups[2])
            pass
        
        if (inline_after_grouping):
            for group in self.groups:
                #clones = [self._clone_map[comp.func] for comp in group.inlined_comps]
                #inline_pass_for_comp (self, [self._func_map[f] for f in clones])
                inline_pass_for_comp (self, group.inlined_comps)
            
            LOG (logging.DEBUG, "\n\n")
            LOG (logging.DEBUG, "Compute Objects after Inlining: ")
            for group in self.groups:
                LOG (logging.DEBUG, group.name+" ")
            
        ''' GRAPH UPDATES '''
        # level order traversal of groups
        self._level_order_groups = self.order_group_objs()
        self._groups = self.get_sorted_groups()
            
        for group in self.groups:
            # update liveness of compute objects in each new group
            group.compute_liveness()
            # children map for comps within the group
            group.collect_comps_children()
        self._liveouts = self.collect_liveouts()
        self._liveouts_children_map = self.build_liveout_graph()

        ''' SCHEDULING '''
        for g in self.groups:
            # alignment and scaling
            align_and_scale(self, g)
            base_schedule(g)
            # grouping, select tile sizes, and tiling
            fused_schedule(self, self._ctx, g, self._param_estimates)
        
        # ***
        log_level = logging.INFO
        LOG(log_level, "\n\n")
        LOG(log_level, "Grouped compute objects:")
        for g in self.groups:
            if 'dpfusion' in self.options:
                s = " Tile sizes: " + str(g.tile_sizes)
            else:
                s = "  "
            LOG(log_level, g.name+s)
        # ***
        
        # group
        self._grp_schedule = schedule_groups(self)
        # comps and poly parts
        for group in self._grp_schedule:
            group.set_comp_and_parts_sched()

        self._liveouts_schedule = schedule_liveouts(self)

        ''' COMPUTE LIVENESS '''
        # liveouts
        self._liveness_map = liveness_for_pipe_outputs(self)
        # groups
        for group in self.groups:
            liveness_for_group_comps(group, group.children_map,
                                     group.comps_schedule)

        ''' STORAGE '''
        # MAPPING
        self.initialize_storage()

        # OPTIMIZATION
        # classify the storage based on type, dimensionality and size
        self._storage_class_map = classify_storage(self)
        # remap logical storage
        self._storage_map = remap_storage(self)

        # ALLOCATION
        self._array_writers_map = create_physical_arrays(self)
        self._free_arrays = create_array_freelist(self)

        # use graphviz to create pipeline graph
        self._pipeline_graph = self.draw_pipeline_graph()

    def get_overlapping_size_for_groups (self, groups, inlined_comps, 
                                         tile_size, _n_buffers):
        LOG (logging.DEBUG, "get_overlapping_size_for_groups for " + \
                            str([g.comps[0].func.name for g in groups]))
        img1 = False
        img2 = False

        for g in groups:
            if (g.comps[0].func.name.find("img1") != -1):
                img1 = True
            elif (g.comps[0].func.name.find("img2") != -1):
                img2 = True
            
        if (img1 and img2):
            #TODO: Special Case for Pyramid Blend. Otherwise, it groups Dx_img1 and
            #Dx_img2, which should not happen since both takes different input images.
            #However, in Harris it is good to group both Ix and Iy group as they
            #take same input images.
            return -1, 0
                
        g = self.create_group (groups, inlined_comps)
        g.set_total_used_size (tile_size)
        g.set_n_buffers (_n_buffers)
        orig_func_map = self.func_map
        _func_map = dict(orig_func_map)
        for comp in g.comps:
            _func_map[comp.func] = comp
        self._func_map = _func_map
        # alignment and scaling
        #try:
        align_and_scale(self, g)
        #except:
        #    return -1
        base_schedule(g)
        # grouping and tiling
        g_all_parts = []
        for comp in g.polyRep.poly_parts:
            g_all_parts.extend(g.polyRep.poly_parts[comp])
        # get dependence vectors between each part of the group and each of its
        # parents' part
        comp_deps = get_group_dep_vecs(self, g, g_all_parts, func_map = _func_map)
        # No point in tiling a group that has no dependencies
        self._func_map = orig_func_map
        det_tile_size = 0
        if len(comp_deps) > 0 and len(g_all_parts) > 1:
            hmax = max( [ p.level for p in g_all_parts ] )
            hmin = min( [ p.level for p in g_all_parts ] )
            slope_min, slope_max = compute_tile_slope(comp_deps, hmax)
            det_tile_size = g.get_tile_sizes (self.param_estimates, slope_min, slope_max, 
                                              g_all_parts, hmax - hmin, _func_map, False, self.multi_level_tiling)
            
            #Restore original body of all functions
            #for comp in g.comps:
                #comp.func.restore_original_body ()
            tile_sizes = g._tile_sizes
            all_slope_invalid = True
            for slope in slope_min:
                if (slope != "*"):
                    all_slope_invalid = False
                    break
            
            if (all_slope_invalid):
                return -1, 0
            
            LOG (logging.DEBUG, "slope_min " + str(slope_min) + " slope_max " + str(slope_max))
            overlap_shifts = [0 for i in range(0, len(slope_min))]
            for i in range(1, len(slope_min)+1):
                if (slope_min[i-1] == '*'):
                    continue
                right = int(math.floor(Fraction(slope_min[i-1][0],
                                                slope_min[i-1][1])))
                left = int(math.ceil(Fraction(slope_max[i-1][0],
                                              slope_max[i-1][1])))
                for h in range(1, get_group_height (g_all_parts)+1):
                    overlap_shifts [i-1] += abs(left * (h)) + abs(right * (h))
                for h in range(1, get_group_height (g_all_parts)+1):
                    LOG (logging.DEBUG, "overlap for i " + str(i) + " and h " +\
                                        str(h) + " = " + str(abs(left * (h)) + \
                                        abs(right * (h))))
            LOG (logging.DEBUG, "overlap_shifts " + str(overlap_shifts))
            overlap_shift = 1
            for i in tile_sizes:
                if ("L1" in str(i)):
                    continue
                if (overlap_shifts[i] != 0 and tile_sizes[i] < g._dim_sizes_part[i]):
                    if ((i-1) in tile_sizes):
                        overlap_shift *= overlap_shifts[i]*(tile_sizes[i-1])
                    else:
                        overlap_shift *= overlap_shifts[i]
                            
            LOG (logging.DEBUG, "overlap_shift " + str(overlap_shift) + \
                                str (" det_tile_size ") + str(det_tile_size))
            return overlap_shift/1.6, det_tile_size
            
        if len (comp_deps) <= 0:
            return 1 << 30, 1
            
        LOG (logging.DEBUG, "Getting Overlap for non stencil? Not Good")
        assert (False)
    
    def pre_grouping_inline_phase (self):
        '''Inline those functions which 
            (i) has only one reference and only one child,
            or 
            (ii) has n cases with n children, each child is associated with
                one case, s.t. no other child is associated with the same case
                and each case has only one reference.
        '''
        self._pre_grouping_inline_groups = []
        for g in list(self.groups):
            if (len(g.children) == 1 and g.comps[0].n_refs () == 1):
                self._pre_grouping_inline_groups += [g.comps[0].func]
            else:
                if (g.inline_in_all_children () and not g.is_pointwise ()):
                    self._pre_grouping_inline_groups += [g.comps[0].func]
        
        inline_pass (self, self._pre_grouping_inline_groups)
    
    @property
    def multi_level_tiling (self):
        return "multi-level-tiling" in self._options
    @property
    def do_inline (self):
        return self._do_inline
    @property
    def dim_reuse (self):
        return self._dim_reuse
    @property
    def param_estimates (self):
        return self._param_estimates
    @property
    def func_map(self):
        return self._func_map
    @property
    def comps(self):
        return self._comps
    @property
    def input_groups(self):
        return self._inp_groups
    @property
    def groups(self):
        return self._groups
    @property
    def name(self):
        return self._name
    @property
    def options(self):
        return self._options
    @property
    def inputs(self):
        return self._inputs
    @property
    def outputs(self):
        return self._outputs
    @property
    def original_graph(self):
        return self._initial_graph
    @property
    def pipeline_graph(self):
        return self._pipeline_graph
    @property
    def get_ordered_comps(self):
        return self._level_order_comps
    @property
    def get_ordered_groups(self):  # <- naming
        return self._level_order_groups
    @property
    def liveouts(self):
        return self._liveouts
    @property
    def liveouts_children_map(self):
        return self._liveouts_children_map
    @property
    def group_schedule(self):
        return self._grp_schedule
    @property
    def liveouts_schedule(self):
        return self._liveouts_schedule
    @property
    def storage_class_map(self):
        return self._storage_class_map
    @property
    def storage_map(self):
        return self._storage_map
    @property
    def liveness_map(self):
        return self._liveness_map
    @property
    def array_writers(self):
        return self._array_writers_map
    @property
    def free_arrays(self):
        return self._free_arrays
        
    def get_tile_sizes_for_group (self, group):
        dim_reuse = {}
        
    def dim_reuse_for_each_group (self):
        for group in self.groups:
            self._dim_reuse [group.comps[0]] = group.get_dimensional_reuse (self.param_estimates)
        
    def create_group(self, groups, inlined_comps):
        LOG (logging.DEBUG, "Pipeline.create_group: inlined_comps " +str([str(k) for k in inlined_comps]))
        comps = []
        for g in groups:
            comps += g.comps
        #for c in comps:
            #c.print_parent ()
            #c.print_children ()
            
        orig_comps = list(comps)
        comp_to_clone = dict ()
        inlined_comp_to_clone = dict ()
        cloned_inlined_comps = []
        will_inline = inlined_comps != []
        func_to_clone_func = dict()
        clone_func_to_func = dict()
        for i in range(len(comps)):
            cloneComp = comps[i].clone(will_inline)
            comp_to_clone [comps[i]] = cloneComp
            func_to_clone_func [comps[i].func] = cloneComp.func
            clone_func_to_func [cloneComp.func] = comps[i].func
            if (comps[i] in inlined_comps):
                cloned_inlined_comps.append (cloneComp)
            comps[i] = cloneComp
        
        for i in range(len(comps)):
            parents = []
            for j in range(len(comps[i].parents)):
                if (comps[i].parents[j] in comp_to_clone):
                    parents.append (comp_to_clone[comps[i].parents[j]])
            comps[i].set_parents (parents)
            children = []
            for j in range(len(comps[i].children)):
                if comps[i].children[j] in comp_to_clone:
                    children.append(comp_to_clone[comps[i].children[j]])
            comps[i].set_children (children)
        
        g = Group (self._ctx, comps, self._param_constraints)
        
        if (will_inline == True):
            for comp in comps:
                comp = comp
                refs = comp.func.getObjects (Reference)
                for ref in refs:
                    if (ref.objectRef in func_to_clone_func):
                        ref._replace_ref_object (func_to_clone_func[ref.objectRef])
                    else:
                        pass
                
        inline_pass_for_comp (self, cloned_inlined_comps, False)
        LOG (logging.DEBUG, "Pipeline.create_group: g after inlining " + str(g))
        
        return g

    def get_size_for_group (self, group):
        size = group.get_total_size ()
        
    def get_parameters(self):
        params=[]
        for group in self.groups:
            params = params + group.getParameters()
        return list(set(params))

    def create_compute_objects(self):
        funcs, parents, children = \
            get_funcs_and_dep_maps(self.outputs)
        comps = []
        func_map = {}
        for func in funcs:
            output = False
            if func in self.outputs:
                output = True
            comp = ComputeObject(func, output)
            func_map[func] = comp
            comps.append(comp)

        # set parents, children information
        for func in func_map:
            comp = func_map[func]
            # set parents
            comp_parents = [func_map[p_func] for p_func in parents[func]]
            comp.set_parents(comp_parents)
            # set children
            comp_children = [func_map[c_func] for c_func in children[func]]
            comp.set_children(comp_children)

        for inp in self._inputs:
            inp_comp = ComputeObject(inp)
            inp_comp.set_parents([])
            inp_comp.set_children([])
            func_map[inp] = inp_comp

        return func_map, comps

    def get_weights (self):
        for g in self.groups():
            g_size = g.get_size_for_each_dim (self._param_estimates)
            children_size = []
            
            for child in g.children ():
                children_size.append (g.get_size_for_each_dim (self._param_estimates))
            
        
    def order_compute_objs(self):
        parent_map = {}
        for comp in self.comps:
            parent_map[comp] = comp.parents
        order = level_order(self.comps, parent_map)
        for comp in order:
            comp.set_level(order[comp])
        return order

    def order_group_objs(self):
        parent_map = {}
        for group in self.groups:
            parent_map[group] = group.parents
        order = level_order(self.groups, parent_map)
        return order

    def get_sorted_comps(self):
        sorted_comps = get_sorted_objs(self._level_order_comps, True)
        return sorted_comps

    def get_sorted_groups(self):
       
        sorted_groups = get_sorted_objs(self._level_order_groups)
        
        return sorted_groups

    def build_initial_groups(self):
        """
        Place each compute object of the pipeline in its own Group, and set the
        dependence relations between the created Group objects.
        """

        # initial groups for inputs
        inp_groups = {}
        for inp_func in self.inputs:
            inp_comp = self.func_map[inp_func]
            inp_groups[inp_func] = \
                pipe.Group(self._ctx, [inp_comp], self._param_constraints)
            # do not add input groups to the list of pipeline groups
        self._inp_groups = inp_groups

        # initial groups for functions
        comps = self.comps
        groups = []
        for comp in comps:
            group = Group(self._ctx, [comp], self._param_constraints)
            # add to the list of pipeline groups
            groups.append(group)

        for group in groups:
            group.find_and_set_parents()
            group.find_and_set_children()

        return groups

    def draw_pipeline_graph(self):
        gr = pgv.AGraph(strict=False, directed=True)

        # TODO add input nodes to the graph
        for i in range(0, len(self.groups)):
            sub_graph_nodes = [comp.func.name for comp in self.groups[i].comps]
            for comp in self.groups[i].comps:
                colour_index = self.storage_map[comp]
                # comp's array mapping
                if comp.is_tstencil_type:
                    node_colour = X11Colours.colour(colour_index[1]) + ";0.5:" + \
                        X11Colours.colour(colour_index[0])
                else:
                    node_colour = X11Colours.colour(colour_index)

                # liveout or not
                node_style = 'rounded,'
                if comp.is_liveout:
                    node_style += 'bold,'
                    node_style += 'filled,'
                    node_shape = 'doublecircle'
                    node_fillcolor = node_colour
                    node_colour = ''
                else:
                    node_style += 'filled,'
                    node_shape = 'box'
                    node_fillcolor = ''

                gr.add_node(comp.func.name,
                    fillcolor=node_fillcolor,
                    color=node_colour,
                    style=node_style,
                    gradientangle=90,
                    shape=node_shape)

            # add group boundary
            gr.add_subgraph(nbunch = sub_graph_nodes,
                            name = "cluster_" + str(i),
                            label=str(self.group_schedule[self.groups[i]]),
                            style="dashed,rounded,")

        for comp in self.comps:
            for p_comp in comp.parents:
                gr.add_edge(p_comp.func.name, comp.func.name)

        gr.layout(prog='dot')
        return gr

    def generate_code(self, is_extern_c_func=False,
                            are_io_void_ptrs=False):

        """
        Code generation for the entire pipeline starts here.

        Flags:

        1. "is_extern_c_func"
        (*) True => function declaration generated with ' extern "C" ' string
                    (used when dynamic libs are needed for python wrapping)
        (*) False => normal C function declaration


        2. "are_io_void_ptrs"
        (*) True => all inputs and outputs of the pipeline are expected, by the
                    C function declaration, to be passed as 'void *'
                    (used when dynamic libs are needed for python wrapping)
        (*) False => inputs and outputs are to be passed as pointers of their
                     data type. E.g: 'float *'
        """

        return generate_code_for_pipeline(self,
                                          is_extern_c_func,
                                          are_io_void_ptrs)

    '''
    Pipelne graph operations
    '''

    def drop_comp(self, comp):
        # if the compute object is a child of any other
        if comp.parents:
            for p_comp in comp.parents:
                p_comp.remove_child(comp)
        # if the compute object is a parent of any other
        if comp.children:
            for c_comp in comp.children:
                c_comp.remove_parent(comp)
        # remove comp_obj
        self._comps.remove(comp)
        func = comp.func
        self._func_map.pop(func)

        return

    def add_group(self, group):
        """
        add a new group to the pipeline
        """
        if not group.comps:
            return

        self._groups.append(group)

        group.find_and_set_parents()
        group.find_and_set_children()

        # add group as child for all its parents
        for p_group in group.parents:
            p_group.add_child(group)
        # add group as parent for all its children
        for c_group in group.children:
            c_group.add_parent(group)

        return

    def drop_group(self, group):
        """
        drop the group from the pipeline
        """
        # if group is a child of any other group
        if group.parents:
            for p_group in group.parents:
                p_group.remove_child(group)
        # if group is a parent of any other group
        if group.children:
            for c_group in group.children:
                c_group.remove_parent(group)
        for comp in group.comps:
            comp.unset_group()
        self._groups.remove(group)

        return

    def merge_groups(self, g1, g2):
        # Get comp objects from both groups
        comps = g1.comps + g2.comps
        comps = list(set(comps))
       
        self.drop_group(g1)
        self.drop_group(g2)
        # Create a new group
        merged = Group(self._ctx, comps,
                       self._param_constraints)

        self.add_group(merged)
        
        
        return merged

    def replace_group(self, old_group, new_group):
        # if old_group has any child
        if old_group.children:
            for child in old_group.children:
                child.add_parent(new_group)
                new_group.add_child(child)
        # if old_group has any parent
        if old_group.parents:
            for parent in old_group.parents:
                parent.add_child(new_group)
                new_group.add_parent(parent)

        # replace old_group with new_group in groups list
        comp = old_group.comps[0]
        self.drop_group(old_group)
        comp.set_group(new_group)

        self._groups.append(new_group)

        return
    
    def make_func_independent_for_comp(self, comp_a, comp_b):
        """
        makes func_b independent of func_b and updates parent children
        relations in the graph structure
        [ assumes that func_b is a child of func_a, and that func_a is inlined
        into func_b ]
        """
        #Group should be same, since, these computations have been merged
        assert (comp_a.group == comp_b.group)
        assert (comp_b in comp_a.children)
        parent = comp_a.group
        if parent:
            # remove relation between a and b
            comp_a.remove_child(comp_b)
            comp_b.remove_parent(comp_a)
            for p in comp_a.parents:
                if p in parent.comps:
                    p.add_child(comp_b)
                    p.remove_child (comp_a)
            
            parents_of_b = comp_b.parents
            for p in comp_a.parents:
                if (p in parent.comps):
                    parents_of_b.append(p)
            comp_b.set_parents (list(set(parents_of_b)))
            
    def make_func_independent(self, func_a, func_b):
        """
        makes func_b independent of func_b and updates parent children
        relations in the graph structure
        [ assumes that func_b is a child of func_a, and that func_a is inlined
        into func_b ]
        """
        comp_a = self.func_map[func_a]
        comp_b = self.func_map[func_b]
        group_a = comp_a.group
        group_b = comp_b.group
       
        if comp_a.parents:
            parents_of_a = comp_a.parents
            parents_of_b = comp_b.parents
            parents_of_grp_a = group_a.parents
            parents_of_grp_b = group_b.parents

            # remove relation between a and b
            comp_a.remove_child(comp_b)
            group_a.remove_child(group_b)

            parents_of_b.remove(comp_a)
            parents_of_grp_b.remove(group_a)

            # new parents list for b
            # compute object
            parents_of_b.extend(parents_of_a)
            parents_of_b = list(set(parents_of_b))
            # group object
            parents_of_grp_b.extend(parents_of_grp_a)
            parents_of_grp_b = list(set(parents_of_grp_b))

            comp_b.set_parents(parents_of_b)
            group_b.set_parents(parents_of_grp_b)

            # new children list for parents_of_b
            for p_comp in parents_of_b:
                p_comp.add_child(comp_b)
            for p_group in parents_of_grp_b:
                p_group.add_child(group_b)
        
        return

    def __str__(self):
        return_str = "Final Group: " + self._name + "\n"
        for s in self._groups:
            return_str = return_str + s.__str__() + "\n"
        return return_str

    def collect_liveouts(self):
        liveouts = [comp for group in self.groups \
                           for comp in group.liveouts]
        return liveouts

    def build_liveout_graph(self):
        liveouts = self.liveouts
        children_map = {}
        for comp in liveouts:
            g_liveouts = []
            if comp.children:
                # collect groups where comp is livein
                livein_groups = [child.group for child in comp.children]
                # collect liveouts of these groups
                for g in livein_groups:
                    g_liveouts += g.liveouts
            if g_liveouts:
                children_map[comp] = g_liveouts
        return children_map

    def initialize_storage(self):
        # ***
        log_level = logging.DEBUG-1
        LOG(log_level, "Initializing Storage ...")

        for func in self.func_map:
            comp = self.func_map[func]
            typ = comp.func.typ
            ndims = comp.func.ndims
            part_map = comp.group.polyRep.poly_parts
            dim_sizes = []
            # 1. Input Images
            # 2. Group Live-Outs
            # 3. Not a scratchpad  (maybe Reduction)
            reduced_dims = [ -1 for i in range(0, ndims) ]
            is_scratch = [ False for i in range(0, ndims) ]
            if comp.is_image_typ or comp.is_liveout or comp not in part_map:
                interval_sizes = comp.size
            # 4. Scratchpads
            else:
                for part in part_map[comp]:
                    for i in range(0, ndims):
                        if i in part.dim_scratch_size:  # as a key
                            reduced_dims[i] = max(reduced_dims[i],
                                                  part.dim_scratch_size[i])
                            is_scratch[i] = True

                for i in range(0, ndims):
                    dim_sizes.append(reduced_dims[i])

                interval_sizes = comp.compute_size(dim_sizes)
#HERE: no element of is_scratch is set to true
            comp.set_scratch_info(is_scratch)

            storage = Storage(typ, ndims, interval_sizes)
            comp.set_orig_storage_class(storage)

            # ***
            LOG(log_level, "  "+comp.func.name)
            LOG(log_level, "    "+str(storage))

        return

