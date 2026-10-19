"""Run a bounded join diagnostic; restore the production menu on every exit."""
from pathlib import Path
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
game = root / 'bin/Debug'
address = sys.argv[1] if len(sys.argv) > 1 else '45.88.229.9:28985'
if any(c not in '0123456789.:' for c in address):
    raise SystemExit('Use a numeric IPv4:port address')
paths = [game / 'main/ui_mp/scriptmenus/main.menu', game / 'ui_mp/scriptmenus/main.menu']
original = [p.read_bytes() for p in paths]
try:
    for path in paths:
        lines = path.read_text().splitlines()
        for index, line in enumerate(lines):
            if line.strip().startswith('onOpen '):
                command = (f' execOnDvarIntValue ui_joinProbe 1 "set ui_joinProbe 0; '
                           f'connect {address}; wait 6000; disconnect; quit"; ')
                lines[index] = line.rsplit('}', 1)[0] + command + '}'
                break
        path.write_text('\n'.join(lines) + '\n')
    process = subprocess.Popen([str(game / 'KisakCOD-mp.exe'), '+set', 'r_fullscreen', '0',
                                '+set', 'logfile', '2', '+set', 'developer', '1',
                                '+set', 'ui_joinProbe', '1'], cwd=game)
    try:
        print('exit', process.wait(timeout=40))
    except subprocess.TimeoutExpired:
        process.terminate()
        process.wait(timeout=10)
        print('Diagnostic deadline reached')
finally:
    for path, data in zip(paths, original):
        path.write_bytes(data)
    destination = root / 'diagnostics/network/join-latest.log'
    destination.write_bytes((game / 'main/console_mp.log').read_bytes())
    print(destination)

