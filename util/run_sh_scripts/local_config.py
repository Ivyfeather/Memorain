import os.path as osp
import platform

cpt_dirs = {
    2006: {
        'ARM': None,
        'RISCV': None,
    },
    2017: {
        'ARM': None,
        'RISCV': osp.expanduser('/home/share/shrink_spec2017_simpoint_cpts'),
        # 'RISCV': osp.expanduser('/home/chenxi/spec2017_simpoint_cpts'),
    },
}

stats_base_dir = osp.expanduser('~/gem5-results-2017')

total_cores = 4

dev_stats_base_dir = osp.expanduser('/vulnerable/zyy')

branch_outcome_dir = osp.expanduser('/home/share/spec2017_branch_outcome')

simpoint_base_dir = osp.expanduser('~/gem5-results')
n_tasks = 1
spare_cores = 1

cores_per_task = (total_cores - spare_cores) // n_tasks


tag_dict = {
        'xiangshan-05': '105',
        'xiangshan-04': '104',
        '135': '135',
        'ubuntu-7700': '130',
        }

machine_tag = tag_dict[platform.node()]


if __name__ == '__main__':
    print(platform.node())
