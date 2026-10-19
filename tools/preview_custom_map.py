"""Bounded in-engine custom-map smoke test; never leaves menu instrumentation installed."""
from pathlib import Path
import re
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
game = root / 'bin/Debug'
name = sys.argv[1] if len(sys.argv) > 1 else 'mp_skid'
if not re.fullmatch(r'[A-Za-z0-9_-]{1,59}', name):
    raise SystemExit('Invalid map name')
paths = [game/'main/ui_mp/scriptmenus/main.menu', game/'ui_mp/scriptmenus/main.menu']
original = [p.read_bytes() for p in paths]
try:
    for path in paths:
        lines = path.read_text().splitlines()
        for i, line in enumerate(lines):
            if line.strip().startswith('onOpen '):
                command = (' execOnDvarIntValue ui_mapProbe 1 "set ui_mapProbe 0"; '
                           ' uiScript FrontendHostMapTest; '
                           f' exec "wait 1800; screenshot cu_map_{name}; disconnect; wait 60; quit"; ')
                lines[i] = line.rsplit('}',1)[0] + command + '}'
                break
        path.write_text('\n'.join(lines)+'\n')
    process = subprocess.Popen([str(game/'KisakCOD-mp.exe'), '+set', 'r_fullscreen', '0',
                               '+set', 'logfile', '2', '+set', 'ui_mapProbe', '1',
                               '+set', 'ui_hostMapSmoke', '1', '+set', 'ui_hostMapName', name], cwd=game)
    try:
        print('exit', process.wait(timeout=55))
    except subprocess.TimeoutExpired:
        process.terminate()
        process.wait(timeout=10)
        print('Map diagnostic deadline reached')
finally:
    for path, data in zip(paths, original): path.write_bytes(data)
    target = root/'diagnostics/maps'
    target.mkdir(parents=True, exist_ok=True)
    (target/(name+'.log')).write_bytes((game/'main/console_mp.log').read_bytes())
