import json
import local_config as lc
import common as c
from pathlib import Path

class SpecCommands:
    def __init__(self, cmd_json):
        with open(cmd_json) as f:
            self.cmds = json.load(f)
    def get(self, task: str):
        if task in self.cmds:
            return self.cmds[task]

        similar = False
        for t in self.cmds:
            if t.startswith(task):
                similar = True
                print(f'{task} not found, {t} is available')

        if not similar:
            print(f'Unknown task: {task}')

    def get_all(self):
        return self.cmds

    def get_local(self):
        filtered = {}

        base_path = Path(__file__).parent
        list_file_path = (base_path / f'benchmark_list/spec2017_on_{lc.machine_tag}.txt').resolve()

        local_benchmarks = c.get_benchmarks(list_file_path)
        for task, cmd in self.cmds.items():
            matched = False
            for b in local_benchmarks:
                if task.startswith(b):
                    matched = True
            if matched:
                filtered[task] = cmd
        return filtered


class Spec17Commands(SpecCommands):
    def __init__(self):
        base_path = Path(__file__).parent
        cmd_file_path = (base_path / 'benchmark_list/spec2017_cmds.json').resolve()
        SpecCommands.__init__(self, cmd_file_path)


if __name__ == '__main__':
    spec = Spec17Commands()
    print(spec.get('gcc_pp_O2'))
    print(spec.get_all())

