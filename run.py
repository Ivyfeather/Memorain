import argparse
import os
import sh
import faulthandler;faulthandler.enable()

gem5 = 'build/RISCV/gem5.opt'
config = 'configs/example/se.py'

wkld_set = {
    "hello":"tests/test-progs/hello/bin/riscv/linux/hello",
    "insttest":"tests/test-progs/insttest/bin/riscv/linux-rv64i/insttest",
    "test":"tests/test-progs/bench/test",
    "stack":"tests/test-progs/stack-print/bin/riscv/stack-print",
    "stream":"tests/test-progs/stream/stream_c.exe"
    }

def add_options(parser):
    parser.add_argument('--dramsim', action='store_true',
                        help='Use DRAMsim3')
    parser.add_argument('--caches', action='store_true',
                        help='Use L1 and L2 cache')
    parser.add_argument("-n", "--num-cpus", type=int, default=1)
    parser.add_argument("-w","--workload",type=str)
    parser.add_argument("--cpu-type",type=str)
    parser.add_argument("--debug-flag",type=str)
    parser.add_argument("--other",type=str)
    parser.add_argument("--l2cache",action='store_true')
    parser.add_argument("-R",action='store_true',help="Redirect output to file")
    parser.add_argument("-M","--memobj",action='store_true',help="use memobj")
    parser.add_argument("-T","--core_tags",type=str)

def get_workloads(wkld_str):
    wklds = []
    wkld_split = wkld_str.split("-")
    for i, name in enumerate(wkld_split):
        # like 4-hello, means hello-hello-hello-hello
        if(name.isdigit()):
            wkld = wkld_split[i+1]
            for cnt in range(int(name)):
                wklds.append(wkld_set[wkld])
        else:
            wklds.append(wkld_set[name])
    return wklds

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    add_options(parser)

    args = parser.parse_args()
    options = []

    if args.debug_flag:
        options.append('--debug-flag={}'.format(args.debug_flag))

    options.append(config)

# options for se.py, following the order in Options.py
    # no ISA
    if args.num_cpus != 1:
        options.append('--num-cpus={}'.format(args.num_cpus))

    if args.core_tags:
        assert(args.num_cpus == len(args.core_tags.split(",")))
        options.append("--core_tags=" + args.core_tags)

    if args.dramsim:
        options.append('--mem-type=DRAMsim3')
            #  '--dramsim3-ini=/home/chenxi/gem5_dramsim/ext/dramsim3/'
            #  'DRAMsim3/configs/DDR4_8Gb_x4_2133.ini')

    if args.caches:
        options.append('--caches')

    if args.l2cache:
        options.append('--l2cache')

    if args.memobj:
        options.append('--use_memobj')

# Common Options
    if args.cpu_type:
        options.append('--cpu-type={}'.format(args.cpu_type))

# SE Options
    if args.workload:
        wklds = get_workloads(args.workload)
        print(wklds)
        options.append("--cmd='{}'".format(";".join(wklds)))
    else:
        options.append('--cmd={}'.format(wkld_set["hello"]))

    if args.other:
        options.append(args.other)

    if args.R:
        options.append("> run_out.log 2>&1")

    cmd = gem5 + " " + " ".join(options)
    print(cmd)
    os.system(cmd)
