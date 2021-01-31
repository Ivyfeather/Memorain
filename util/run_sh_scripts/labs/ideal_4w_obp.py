#!/usr/bin/env python3

import sys
sys.path.append('../')

from os.path import join as pjoin
from multiprocessing import Pool
import common as c
import local_config as lc

num_thread = lc.cores_per_task
window_size = 192 * 1

lbuf_on = True
obp = True
full = False
debug = False
simpoint_list = [0, 1, 2]
#simpoint_list = [1]
benchmark_list_file = '../all_function_spec2017.txt'
#benchmark_list_file = './tmp.txt'

if full:
    d = '_f'
else:
    d = '_s'
if obp:
    obp_suffix = '_o'
else:
    obp_suffix = '_p'
if lbuf_on:
    lbuf_suffix = '_lbuf'
else:
    lbuf_suffix = '_nbuf'

# config = f'ideal_8w{obp_suffix}'
config = f'ideal_trad_4w{lbuf_suffix}{obp_suffix}'
outdir = f'{c.stats_base_dir}/{config}{d}/'

def main():
    g5_configs = []

    dict_options = {
            '--num-IQ': window_size,
            '--o3-core-width': 4,

            '--branch-trace-file': 'useless_branch.protobuf.gz',
            }

    if obp:
        dict_options['--use-bp'] = 'OracleBP'
    else:
        dict_options['--use-bp'] = 'ZPerceptron'

    binary_options= [
            '--check-outcome-addr',
            '--branch-trace-en',
            ]
    if lbuf_on:
        binary_options.append(
                '--enable-loop-buffer',
                )

    with open(benchmark_list_file) as f:
        for line in f:
            if not line.startswith('#'):
                for cpt_id in simpoint_list:
                    benchmark = line.strip()
                    task = benchmark + '_' + str(cpt_id)
                    g5_config = c.G5Config(
                        benchmark=benchmark,
                        window_size=window_size,
                        bmk_outdir=pjoin(outdir, task),
                        cpt_id=cpt_id,
                        arch='RISCV',
                        full=full,
                        full_max_insts=220 * 10**6,
                        debug=debug,
                        # panic_tick=320437123372000,
                        debug_flags=[
                            'LoopBufferStack',
                            'Fetch',
                            'LoopBuffer',
                            ],
                        func_id=config,
                    )
                    g5_config.add_options(binary_options)
                    g5_config.update_options(dict_options)
                    g5_configs.append(g5_config)

    if num_thread > 1:
        p = Pool(num_thread)
        p.map(c.run_wrapper, g5_configs)
    else:
        g5_configs[0].check_and_run()


if __name__ == '__main__':
    main()

