from __init__ import *

import sys
sys.path.insert(0, ROOT+"/apps/python/")

from cpp_compiler import *
from constructs import *
from builder import create_lib,build_mfd
from compiler import *
import tuner
from polymage_mfd import minifluxdiv

def auto_tune(app_data):
    pipe_data = app_data['pipe_data']

    app_name = app_data['app_name']
    pipe_name = app_data['app']
    pipe_func_name = "pipeline_"+pipe_name

    out_mfd, inline_func = minifluxdiv(pipe_data)
    live_outs = [out_mfd]
    
    B = pipe_data['B']
    N = pipe_data['N']

    boxes = app_data['boxes']
    cells = app_data['cells']

    param_estimates =   [(B, boxes), (N, cells)]
    param_constraints = [ Condition(B, "==", boxes), \
                          Condition(N, "==", cells) ]

    dst_path = "/tmp"

    #group_size_configs = [3, 5, 7, 9, 10]
    group_size_configs = [4]
    #group_size_configs = [1]

    tile_size_configs = []
    tile_size_configs.append([7, 7, 128])
    
    # relative path to root directory from app dir
    ROOT = app_data['ROOT']
    opts = []
    if app_data['early_free']:
        opts += ['early_free']
    if app_data['optimize_storage']:
        opts += ['optimize_storage']
    if app_data['pool_alloc']:
        opts += ['pool_alloc']
    if app_data['dpfuse']:
        opts += ['dpfuse']

    gen_compile_string(app_data)
    cxx_string = app_data['cxx_string']

    # Generate Variants for Tuning
    # ============================

    gen_config = {"_tuner_app_name": app_name,
                  "_tuner_live_outs": live_outs,
                  "_tuner_param_constraints": param_constraints, #optional
                  "_tuner_param_estimates": param_estimates, #optional
                  "_tuner_tile_size_configs": tile_size_configs, #optional
                  "_tuner_group_size_configs": group_size_configs, #optional
                  "_tuner_opts": opts, #optional
                  "_tuner_dst_path" : dst_path, # optional
                  "_tuner_cxx_string" : cxx_string, # optional
                  "_tuner_root_path" : ROOT, # needed if pool_alloc is set
                  "_tuner_debug_flag": True, # optional
                  "_tuner_opt_datadict": app_data
                 }

    _tuner_src_path, _tuner_configs_count, _tuner_pipe = \
        tuner.generate(gen_config)

    # '''
    # _tuner_src_path = '/tmp/PolycNUUEYoLt2Mage'
    # _tuner_configs_count = 75
    # _tuner_pipe = buildPipeline(live_outs)
    # '''

    mfd_data = app_data['mfd_data']
    IN = mfd_data['IN']
    OUT = mfd_data['OUT']

    pipe_args = {}
    pipe_args['B'] = boxes
    pipe_args['N'] = cells
    pipe_args['box'] = IN 
    pipe_args['sums'] = OUT 


    # Execute the generated variants
    # ==============================

    exec_config = {"_tuner_app_name": app_name,
                   "_tuner_pipe": _tuner_pipe,
                   "_tuner_pipe_arg_data": pipe_args,
                   "_tuner_src_path": _tuner_src_path, # optional
                   "_tuner_configs_count": _tuner_configs_count, # optional
                   "_tuner_omp_threads": 4, # optional
                   "_tuner_nruns": 10, # optional
                   "_tuner_debug_flag": True, # optional
                   #"_tuner_custom_executor": minimal_exec_mg,
                   "_tuner_app_data": app_data
                  }

    tuner.execute(exec_config)
