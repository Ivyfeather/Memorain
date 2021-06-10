#!/bin/bash
#run_bzip2.sh
set -x
gem5path=/home/chenxi/Memorain
spec2006path=/home/share/cpu2006-arm-riscv/benchspec/CPU2006
outdir=/home/chenxi/spec2006Trace

bench=401.bzip2
ben_suffix=exe
exe=bzip2_base.gcc-riscv64-gnu-8.2.0
options=$spec2006path/$bench/data/test/input/control

# bench=470.lbm
# ben_suffix=exe
# exe=lbm_base.gcc-riscv64-gnu-8.2.0
# options="20 reference.dat 0 1 /home/share/cpu2006-arm-riscv/benchspec/CPU2006/470.lbm/data/test/input/100_100_130_cf_a.of"

# bench=403.gcc
# ben_suffix=exe
# exe=gcc_base.gcc-riscv64-gnu-8.2.0
# options=$spec2006path/$bench/data/test/input/cccp.i


# $gem5path/build/RISCV/gem5.opt \
# 	--debug-flags=MemoryAccess \
# 	--outdir=$outdir --debug-file=401.bzip2.out \
$gem5path/build/RISCV/gem5.opt \
	--outdir=$outdir \
	--debug-flag=SoloTrace \
	$gem5path/configs/example/se.py \
	-c "$spec2006path/$bench/$ben_suffix/$exe" \
	-o "$options" \
	-i "" \
	--num-cpus=1 \
	--cpu-type=DerivO3CPU \
	--caches \
	--l2cache \
	-I 50000000 \
	# --mem-type=DRAMsim3
	# --mem-size=8192MB
