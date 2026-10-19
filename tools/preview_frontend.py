from pathlib import Path
import subprocess,sys
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]; game=ROOT/'bin/Debug'
menu=sys.argv[1] if len(sys.argv)>1 else 'createserver'
paths=[game/'main/ui_mp/scriptmenus/main.menu',game/'ui_mp/scriptmenus/main.menu']
original=[p.read_bytes() for p in paths]
shot='cu_'+menu
smoke='--smoke' in sys.argv
restart='--restart' in sys.argv
try:
 for p in paths:
  s=p.read_text(); lines=s.splitlines()
  for i,line in enumerate(lines):
   if line.strip().startswith('onOpen '):
    if restart:
     diagnostic=f' open {menu}; execOnDvarIntValue ui_frontendPreviewStage 1 "wait 180; screenshot {shot}; set ui_frontendPreviewStage 2; vid_restart"; execOnDvarIntValue ui_frontendPreviewStage 2 "set ui_frontendPreviewStage 0; wait 180; screenshot {shot}_restart; wait 120; quit"; '
    else:
     diagnostic=('uiScript FrontendSmokeTest; ' if smoke else '')+f' open {menu}; exec "wait 180; screenshot {shot}; wait 120; quit"; '
    lines[i]=line.rsplit('}',1)[0]+diagnostic+'}'
    break
  p.write_text('\n'.join(lines)+'\n')
 p=subprocess.Popen([str(game/'KisakCOD-mp.exe'),'+set','r_fullscreen','0','+set','logfile','2','+set','ui_frontendSmoke','1' if smoke else '0','+set','ui_frontendPreviewStage','1' if restart else '0'],cwd=game)
 try: print('exit',p.wait(timeout=45))
 except subprocess.TimeoutExpired:
  p.terminate();p.wait(timeout=10);raise SystemExit('Preview timed out')
 target=ROOT/'assets/frontend/previews';target.mkdir(exist_ok=True)
 source=game/'main/screenshots'/(shot+'.tga')
 if p.returncode: raise SystemExit('Game failed with exit code '+str(p.returncode))
 if restart: source=game/'main/screenshots'/(shot+'_restart.tga')
 if source.exists():
  Image.open(source).save(target/(menu+'.png'));print(target/(menu+'.png'))
finally:
 for p,data in zip(paths,original): p.write_bytes(data)
