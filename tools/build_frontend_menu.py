from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
parts=['''// COD Unlimited front page. Editable independently of the stock fastfiles.
menudef
{
 name "main"
 fullscreen 1
 rect 0 0 640 480
 focuscolor 0.94 0.98 0.91 1
 onOpen { uiScript addPlayerProfiles; uiScript openMenuOnDvar com_playerProfile "" player_profile; uiScript openMenuOnDvarNot ui_playerProfileCount 1 player_profile; uiScript stopRefresh; setdvar ui_showEndOfGame 0; focusfirst; }
 soundLoop "music_mainmenu_mp"
''']
def item(name,rect,extra,align='1 0'):
 parts.append(f' itemDef\n {{\n  name "{name}"\n  rect {rect} {align}\n  visible 1\n{extra}\n }}\n')
def panel(name,rect,color,align='1 0'):
 item(name,rect,f'  style 1\n  backcolor {color}\n  decoration',align)
def label(name,rect,text,scale='.18',color='.65 .69 .66 1',baseline=14,font=0,align='1 0'):
 item(name,rect,f'  text "{text}"\n  textfont {font}\n  textscale {scale}\n  textaligny {baseline}\n  forecolor {color}\n  decoration',align)
item('background','0 0 640 480','  style 3\n  background "codunlimited_frontend"\n  forecolor 1 1 1 1\n  decoration','4 4')
panel('sidebar_tint','0 0 283 480','.012 .020 .018 .20')
item('logo','26 18 190 79','  style 3\n  background "codunlimited_logo"\n  forecolor 1 1 1 1\n  decoration')
panel('logo_rule','32 101 23 1','.62 .86 .47 .9')
panel('logo_rule_secondary','57 101 17 1','.24 .36 .25 .5')
rows=[('MULTIPLAYER','Play online or on LAN','multiplayer','open pc_join_unranked;'),('LOCAL PLAY','Create a local server','local','close self; open createserver;'),('MODS','Browse and manage mods','mods','close self; open mods_menu;'),('SETTINGS','Video, audio and game options','settings','open main_options;'),('EXTRAS','Rank, challenges and progression','extras','open menu_challenges;'),('QUIT','Exit to desktop','quit','open quit_popmenu;')]
for n,(title,desc,icon,action) in enumerate(rows):
 y=122+n*47
 focus=f'setitemcolor nav_buttons backcolor .035 .052 .047 .72; setitemcolor nav_buttons bordercolor .20 .25 .23 .7; setitemcolor nav_edges backcolor .64 .9 .49 0; setitemcolor nav_icons forecolor .72 .77 .74 1; setitemcolor button_{n} backcolor .13 .22 .115 .83; setitemcolor button_{n} bordercolor .40 .57 .33 .85; setitemcolor edge_{n} backcolor .64 .9 .49 1; setitemcolor icon_{n} forecolor .72 .94 .59 1; play mouse_over;'
 item('button_'+str(n),f'28 {y} 236 40',f'''  group "nav_buttons"
  type 1
  style 1
  backcolor .035 .052 .047 .72
  border 1
  bordersize .5
  bordercolor .20 .25 .23 .7
  text "{title}"
  textfont 2
  textscale .29
  textalignx 38
  textaligny 17
  forecolor .88 .91 .88 1
  onFocus {{ {focus} }}
  action {{ play mouse_click; {action} }}''')
 item('edge_'+str(n),f'28 {y} 2 40','  group "nav_edges"\n  style 1\n  backcolor .64 .9 .49 0\n  decoration')
 item('icon_'+str(n),f'40 {y+10} 17 17',f'  group "nav_icons"\n  style 3\n  background "codunlimited_icon_{icon}"\n  forecolor .72 .77 .74 1\n  decoration')
 label('description_'+str(n),f'66 {y+20} 186 14',desc,'.165','.57 .63 .59 1',11)
 label('chevron_'+str(n),f'249 {y+9} 10 18','>','.23','.45 .55 .46 .8',12)
panel('footer_tint','0 453 640 27','.01 .015 .013 .83','4 4')
panel('footer_rule','0 453 640 .5','.28 .34 .29 .5','4 4')
label('footer','28 459 240 15','COD UNLIMITED   |   v0.0.1','.17','.55 .61 .55 1',11)
label('mode_caption','-150 459 130 15','MULTIPLAYER','.17','.55 .61 .55 1',11,align='3 0')
parts.append('}\n')
s=''.join(parts)
s=s.replace('focusfirst; }', 'focusfirst; setitemcolor button_0 backcolor .13 .22 .115 .83; setitemcolor button_0 bordercolor .40 .57 .33 .85; setitemcolor edge_0 backcolor .64 .9 .49 1; setitemcolor icon_0 forecolor .72 .94 .59 1; }')
for dest in ['bin/Debug/main/ui_mp/scriptmenus/main.menu','bin/Debug/ui_mp/scriptmenus/main.menu']:
 p=ROOT/dest
 if not p.with_suffix('.menu.previous').exists(): p.with_suffix('.menu.previous').write_bytes(p.read_bytes())
 p.write_text(s)
print('Updated main menu: logo, artwork, reference-style buttons, icons, version 0.0.1')
