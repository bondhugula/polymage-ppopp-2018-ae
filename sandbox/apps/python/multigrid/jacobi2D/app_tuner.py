import sys

from polymage_vcycle import vCycle
from polymage_wcycle import wCycle
from execMG import minimal_exec_mg
from constructs import *

from compiler import *
import tuner

def auto_tune(impipe_dict, data_dict):

    cycle_type = data_dict['cycle']
    if cycle_type == 'V':
        mg = vCycle(impipe_dict, data_dict)
    elif cycle_type == 'W':
        mg = wCycle(impipe_dict, data_dict)

    app_name = data_dict['cycle_name']
    live_outs = [mg]
    n = impipe_dict['n']
    param_estimates = [(n, data_dict['n'])]
    param_constraints = [ Condition(n, '==', data_dict['n']) ]
    dst_path = "/tmp"

    group_size_configs = [3, 5, 7, 9, 11, 13, 15]

    tile_size_configs = []
    tile_size_configs.append([64, 256])
    tile_size_configs.append([64, 128])

    tile_size_configs.append([32, 512])
    tile_size_configs.append([32, 256])
    tile_size_configs.append([32, 128])
    tile_size_configs.append([32, 64])

    tile_size_configs.append([16, 512])
    tile_size_configs.append([16, 256])
    tile_size_configs.append([16, 128])
    tile_size_configs.append([16, 64])

    tile_size_configs.append([8, 512])
    tile_size_configs.append([8, 256])
    tile_size_configs.append([8, 128])
    tile_size_configs.append([8, 64])
    tile_size_configs.append([8, 32])

    opts = ['pool_alloc']


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
                  "_tuner_should_debug": True, # optional
                  "_tuner_opt_datadict": data_dict
                 }

    _tuner_src_path, _tuner_configs_count, _tuner_pipe = \
        tuner.generate(gen_config)


    pipe_arg_dict = {}
    pipe_arg_dict['n'] = data_dict['n']
    pipe_arg_dict['U_'] = data_dict['input_list'][0]
    pipe_arg_dict['F_'] = data_dict['input_list'][1]
    pipe_arg_dict['U_EXACT_'] = data_dict['output_list'][2]

    # Execute the generated variants
    # ==============================

    exec_config = {"_tuner_pipe_arg_dict": pipe_arg_dict,
                   "_tuner_app_name": app_name,
                   "_tuner_pipe": _tuner_pipe,
                   "_tuner_src_path": _tuner_src_path, # optional
                   "_tuner_configs_count": _tuner_configs_count, # optional
                   "_tuner_omp_threads": 16, # optional
                   "_tuner_nruns": 1, # optional
                   "_tuner_should_debug": True, # optional
                   "_tuner_custom_executor": minimal_exec_mg
                  }

    tuner.execute(exec_config)