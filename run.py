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

tracedir="result/trace/"

tracefile_set = {
    "hello" :tracedir+"o3cpu_hello.log",
    "stream":tracedir+"o3cpu_stream.log"
}

spec06path="/home/share/cpu2006-arm-riscv/benchspec/CPU2006/"
inputdir="data/test/input/"
outdir="/home/chenxi/spec2006Trace/"


spec06_id = {
    "perlbench":"400",
    "bzip2":"401",
    "gcc":"403",
    "lbm":"470",
    # "bwaves":"410",
    # "sphinx3":"482",
    "sjeng":"458",
    "namd":"444",
    "cactusADM":"436"
}

spec06_option = {
    "perlbench":spec06path+"400.perlbench/" +inputdir+"makerand.pl",
    "bzip2":    spec06path+"401.bzip2/"     +inputdir+"control",
    "gcc":      spec06path+"403.gcc/"       +inputdir+"cccp.i",
    "lbm":      "20 reference.dat 0 1 "+spec06path+"470.lbm/"+inputdir+"100_100_130_cf_a.of" ,
    # "bwaves":   "",
    # "sphinx3":  spec06path+"482.sphinx3/"+inputdir+"ctlfile "+\
    #     spec06path+"482.sphinx3/"+inputdir+" "+\
    #     spec06path+"482.sphinx3/"+inputdir+"args.an4",
    "sjeng":    spec06path+"458.sjeng/"     +inputdir+"test.txt",
    "namd":     " --input "            +spec06path+"444.namd/data/all/input/namd.input"+" --iterations 1 ",
    "cactusADM":spec06path+"436.cactusADM/" +inputdir+"benchADM.par"
}

def add_options(parser):
    parser.add_argument('--dramsim', action='store_true',
                        help='Use DRAMsim3')
    parser.add_argument('--caches', action='store_true',
                        default=True, help='Use L1 Cache')
    parser.add_argument("-n", "--num-cpus", type=int, default=1)
    parser.add_argument("-w","--workload",type=str)
    parser.add_argument("--cpu-type",type=str,default="DerivO3CPU")
    parser.add_argument("--debug-flag",type=str)
    parser.add_argument("--other",type=str)
    parser.add_argument("--l2cache", action='store_true', default=True)
    parser.add_argument("-R",action='store_true',help="Redirect output to file")
    parser.add_argument("-M","--use_memobj",action='store_true',help="use memobj")
    parser.add_argument("-T","--core_tags",type=str)
    parser.add_argument("-I","--maxinsts",type=int,default=50000000)
    parser.add_argument("--spec06",action="store_true",help="Use Spec2006 Benchmarks")

def get_names(input):
    names = []
    input_split = input.split("-")
    hasdigit = False
    for i, item in enumerate(input_split):
        # like 4-hello, means hello-hello-hello-hello
        if(item.isdigit()):
            name = input_split[i+1]
            names.extend([name for x in range(int(item))])
            hasdigit = True
        # pass hello in 4-'hello'
        elif(hasdigit):
            hasdigit = False
        else:
            names.append(item)
    return names


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
        if args.use_memobj:
            assert args.core_tags, "must assign tags when more than one cpus"
        options.append('--num-cpus={}'.format(args.num_cpus))

    if args.core_tags:
        assert args.num_cpus == len(args.core_tags.split(","))
        options.append("--core_tags=" + args.core_tags)

    if args.dramsim:
        options.append('--mem-type=DRAMsim3')
            #  '--dramsim3-ini=/home/chenxi/gem5_dramsim/ext/dramsim3/'
            #  'DRAMsim3/configs/DDR4_8Gb_x4_2133.ini')

    if args.caches:
        options.append('--caches')

    if args.l2cache:
        options.append('--l2cache')

    if args.use_memobj:
        options.append('--use_memobj')

# Common Options
    
    options.append('--cpu-type={}'.format(args.cpu_type))
    
    options.append('-I {}'.format(args.maxinsts))

    # options.append('--mem-size=8192MB')

# SE Options
    if args.workload and args.spec06:
        benches = get_names(args.workload)
        exes    = [ spec06path + spec06_id[bench] + "." + bench + "/exe/"+\
            bench+"_base.gcc-riscv64-gnu-8.2.0" for bench in benches]
        specin = [ spec06_option[bench] for bench in benches ]
        traces  = [ tracedir+bench+"_o3_ddr3.log" for bench in benches]
        
        options.append("-c '{}'".format(";".join(exes)))
        options.append("-o '{}'".format(";".join(specin)))
        if(args.use_memobj):
            options.append("--paths='{}'".format(",".join(traces)))
    
    elif args.workload:
        names   = get_names(args.workload)
        wklds   = [wkld_set[name] for name in names]
        traces  = [tracefile_set[name] for name in names]
        print(wklds)
        options.append("-c '{}'".format(";".join(wklds)))
        if(args.use_memobj):
            options.append("--paths='{}'".format(",".join(traces)))
    else:
        options.append('-c {}'.format(wkld_set["hello"]))
        if(args.use_memobj):
            options.append('--paths={}'.format(tracefile_set["hello"]))

    if args.other:
        options.append(args.other)

    if args.R:
        options.append("> run_out.log 2>&1")

    cmd = gem5 + " " + " ".join(options)
    print(cmd)
    os.system(cmd)
