#!/usr/bin/env python3

import PIL.Image
import PIL.ImageDraw
import PIL.ImageFont

NB=3
BS=4
BPX=64
M1=1
M2=3
FC='#000000'

fnt1 = PIL.ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf', int(BPX*0.4))
fnt2 = PIL.ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf', int(BPX*0.65))
fnt3 = PIL.ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf', int(BPX))

frame = PIL.Image.new('RGBA', (NB*BS*BPX + BPX + NB*BS*BPX, NB*BS*BPX), color=None)
draw = PIL.ImageDraw.Draw(frame)

def makemat(off):
  for bi in range(NB):
    for bj in range(NB):
      for i in range(BS):
        for j in range(BS):
          if off == 0:
            eff_i = i
            eff_j = j
            eff_bi = bi
            eff_bj = bj
          else:
            eff_i = j
            eff_j = i
            eff_bi = bj
            eff_bj = bi

          y = (bi*BS+i)*BPX
          x = (bj*BS+j)*BPX + off
          c1 = int(255.0/(NB*BS-1)*(eff_bi*BS+eff_i))
          c2 = int(255.0/(NB*BS-1)*(eff_bj*BS+eff_j))
          draw.rectangle(
            [(x, y), (x+BPX, y+BPX)],
            fill=(255, c1, c2),
            outline='black',
            width=M1
          )
          draw.text((x+BPX/2, y+BPX/2), f'{eff_i}{eff_j}', font=fnt1, fill=FC, anchor='mm')
  for bi in range(NB):
    for bj in range(NB):
      if off == 0:
        eff_bi = bi
        eff_bj = bj
      else:
        eff_bi = bj
        eff_bj = bi

      y = bi*BS*BPX
      x = bj*BS*BPX + off
      draw.rectangle(
        [(x, y), (x+BS*BPX, y+BS*BPX)],
        fill=None,
        outline='black',
        width=M2
      )
      c1 = chr(ord('A')+eff_bi)
      c2 = chr(ord('A')+eff_bj)
      draw.text((x+BS*BPX/2, y+BS*BPX/2), f'{c1}{c2}', font=fnt2, fill=FC, anchor='mm')
    draw.rectangle(
      [(off, 0), (off+NB*BS*BPX, NB*BS*BPX)],
      fill=None,
      outline='black',
      width=M2*2
    )

makemat(0)
makemat(NB*BS*BPX + BPX)

draw.text((NB*BS*BPX+BPX/2, NB*BS*BPX/2), "➡", font=fnt3, fill=(0, 0, 0), anchor='mm')

frame.save(f'src/blkinterpret.png')



