from pathlib import Path
from PIL import Image,ImageDraw
import io,struct,math
ROOT=Path(__file__).resolve().parents[1]
SOURCE=ROOT/'assets/frontend'; OUT=ROOT/'bin/Debug/main/images'; OUT.mkdir(parents=True,exist_ok=True)
def iwi(name,image,size):
    image=image.convert('RGBA').resize(size,Image.Resampling.LANCZOS)
    stream=io.BytesIO(); image.save(stream,format='DDS',pixel_format='DXT5')
    dds=stream.getvalue(); assert dds[:4]==b'DDS ' and dds[84:88]==b'DXT5'
    pixels=dds[128:]; total=28+len(pixels)
    # COD4 v6, DXT5, no picmip or mipmaps, clamp U/V, one 2D level.
    header=struct.pack('<3sBBB3h4i',b'IWi',6,13,0xC3,*size,1,*([total]*4))
    (OUT/(name+'.iwi')).write_bytes(header+pixels)
    print(name,size,total)
iwi('codunlimited_frontend',Image.open(SOURCE/'background-v2.png'),(2048,1024))
iwi('codunlimited_logo',Image.open(SOURCE/'logo.png'),(1024,512))
# Deterministic UI pictograms; geometry remains editable in this source.
for name in ['multiplayer','local','mods','settings','extras','quit']:
    im=Image.new('RGBA',(256,256)); d=ImageDraw.Draw(im); c=(255,255,255,255)
    if name=='multiplayer':
        d.ellipse((88,30,148,90),fill=c); d.rounded_rectangle((63,104,176,201),24,fill=c)
        d.ellipse((169,57,211,99),fill=c); d.rounded_rectangle((181,111,232,187),16,fill=c)
        d.ellipse((24,57,66,99),fill=c); d.rounded_rectangle((7,111,58,187),16,fill=c)
    elif name=='local':
        d.polygon([(18,117),(128,26),(238,117),(217,142),(128,69),(39,142)],fill=c)
        d.rectangle((53,119,203,223),fill=c); d.rectangle((107,156,149,224),fill=(0,0,0,0))
    elif name=='mods':
        d.rounded_rectangle((49,56,201,209),8,fill=c)
        d.ellipse((101,22,148,86),fill=c); d.ellipse((169,110,231,159),fill=c)
        d.ellipse((21,110,84,159),fill=(0,0,0,0)); d.ellipse((101,179,149,235),fill=(0,0,0,0))
    elif name=='settings':
        points=[]
        for i in range(48):
            a=math.tau*i/48; r=105 if i%6 in (1,2,3,4) else 82
            points.append((128+math.cos(a)*r,128+math.sin(a)*r))
        d.polygon(points,fill=c); d.ellipse((86,86,170,170),fill=(0,0,0,0)); d.ellipse((108,108,148,148),fill=c)
    elif name=='extras':
        d.ellipse((25,25,231,231),outline=c,width=16)
        d.polygon([(128,43),(148,108),(213,128),(148,148),(128,213),(108,148),(43,128),(108,108)],fill=c)
        d.ellipse((112,112,144,144),fill=(0,0,0,0))
    else:
        d.arc((35,34,221,223),-60,240,fill=c,width=20)
        d.rounded_rectangle((118,17,138,130),8,fill=c)
    im=im.resize((64,64),Image.Resampling.LANCZOS); im.save(SOURCE/('icon-'+name+'.png'))
    iwi('codunlimited_icon_'+name,im,(64,64))

# Thin UI rails and chevrons share the menu's restrained green/gray palette.
for name in ['scroll_track','scroll_thumb','scroll_up','scroll_down','slider_track','slider_thumb']:
    im=Image.new('RGBA',(256,256)); d=ImageDraw.Draw(im)
    pale=(166,185,171,235); green=(165,220,130,255)
    if name=='scroll_track': d.rectangle((122,0,134,255),fill=(89,111,96,145))
    elif name=='scroll_thumb': d.rounded_rectangle((106,0,150,255),8,fill=pale)
    elif name=='scroll_up': d.line([(84,147),(128,103),(172,147)],fill=pale,width=12)
    elif name=='scroll_down': d.line([(84,109),(128,153),(172,109)],fill=pale,width=12)
    elif name=='slider_track':
        d.rectangle((0,112,255,144),fill=(35,52,42,245)); d.rectangle((0,124,255,132),fill=(108,133,114,230))
    else: d.rounded_rectangle((82,32,174,224),12,fill=green)
    im=im.resize((64,64),Image.Resampling.LANCZOS); im.save(SOURCE/(name+'.png'))
    iwi('codunlimited_'+name,im,(64,64))

for name in ['video','audio','controls','network']:
    im=Image.new('RGBA',(256,256));d=ImageDraw.Draw(im);c=(255,255,255,255)
    if name=='video':
        d.rounded_rectangle((22,34,234,177),12,outline=c,width=16);d.rectangle((117,176,139,210),fill=c);d.rounded_rectangle((72,207,184,221),6,fill=c)
    elif name=='audio':
        d.arc((36,24,220,213),180,360,fill=c,width=18);d.rounded_rectangle((25,109,67,212),13,fill=c);d.rounded_rectangle((189,109,231,212),13,fill=c)
    elif name=='controls':
        d.rounded_rectangle((60,20,196,236),66,outline=c,width=16);d.line((128,25,128,112),fill=c,width=12);d.line((64,112,192,112),fill=c,width=12);d.rounded_rectangle((116,48,140,86),10,fill=c)
    else:
        d.line([(128,85),(128,137),(50,137),(50,173)],fill=c,width=13);d.line([(128,137),(206,137),(206,173)],fill=c,width=13)
        for x,y in [(128,50),(50,204),(206,204)]:d.rounded_rectangle((x-35,y-27,x+35,y+27),8,outline=c,width=13)
    im=im.resize((64,64),Image.Resampling.LANCZOS);im.save(SOURCE/('icon-'+name+'.png'));iwi('codunlimited_icon_'+name,im,(64,64))
