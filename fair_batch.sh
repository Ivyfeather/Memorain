# python3 run.py --spec -w=perlbench-bzip2-namd-perlbench -n=4 -M -T=1,2,2,2 > result/fair/fair3.log 2>&1 &
python3 run.py --spec -w=bzip2-gcc-namd-bzip2-cactusADM-lbm -n=6 -M -T=1,2,2,2,2,2 > result/fair/fair4.log 2>&1 &
# python3 run.py --spec -w=perlbench-sjeng-namd-bzip2 -n=4 -M -T=1,2,2,2 > result/fair/fair5.log 2>&1 &

# python3 run.py --spec -w=bzip2-bzip2-perlbench-perlbench -n=4 -M -T=1,2,2,2 > result/fair/fair6.log 2>&1 &
python3 run.py --spec -w=lbm-gcc-lbm-namd-namd-sjeng -n=6 -M -T=1,2,2,2,2,2 > result/fair/fair7.log 2>&1 &
# python3 run.py --spec -w=namd-sjeng-namd-bzip2 -n=4 -M -T=1,2,2,2 > result/fair/fair8.log 2>&1 &

python3 run.py --spec -w=lbm-lbm-lbm-gcc-sjeng-cactusADM -n=6 -M -T=1,2,2,2,2,2 > result/fair/fair9.log 2>&1 &
# python3 run.py --spec -w=cactusADM-sjeng-namd-perlbench -n=4 -M -T=1,2,2,2 > result/fair/fair10.log 2>&1 &
