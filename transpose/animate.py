#!/usr/bin/env python3


import sys
import os
import numpy as np
np.set_printoptions(threshold=sys.maxsize, linewidth=sys.maxsize, suppress=True)
import PIL.Image
import PIL.ImageDraw
import PIL.ImageFont
import colorsys
#import matplotlib.pyplot as plt
#import colorcet as cc
import pkg_resources





#### Matrix state
class Matrix:
  FUTURE = 1
  PRESENT = 2
  PAST = 3
  PADDING = 4

  def __init__(self, address, n, npad):
    self.address = address
    self.n = n
    self.npad = npad
    self.data = np.full((npad, npad), Matrix.PADDING, dtype=np.uint8)
    self.data[:n,:n] = Matrix.FUTURE


#### LRU cache

class Cache:
  def __init__(self, cls, cache_size):
    self.cls = cls # cache line size
    self.cache_size = cache_size # numbere of cache lines
    self.cache = list()

  def touch(self, mat, i, j):
    clid = (mat.address + i*mat.data.shape[1] + j) // self.cls
    if clid in self.cache:
      self.cache.remove(clid)
    self.cache.insert(0, clid)
    while len(self.cache) > self.cache_size:
      del self.cache[-1]

  def age(self, mat, i, j):
    needle = (mat.address + i*mat.data.shape[1] + j) // self.cls
    for i,clid in enumerate(self.cache):
      if clid == needle:
        return i
    return None



#### Visualizer

class Palette:
  def __init__(self):
    self.idx2color = list()
    self.color2idx = dict()

  def _register(self, key):
    if key not in self.color2idx:
      idx = len(self.idx2color)
      self.color2idx[key] = idx
      self.idx2color.append(key)
    return self.color2idx[key]

  def rgb(self, r, g, b):
    assert(0 <= r <= 255)
    assert(0 <= g <= 255)
    assert(0 <= b <= 255)
    key = (int(round(r)), int(round(g)), int(round(b)))
    return self._register(key)

  def hsv(self, h, s, v):
    assert(0 <= h <= 360)
    assert(0 <= s <= 1)
    assert(0 <= v <= 1)
    r,g,b = colorsys.hsv_to_rgb(h/360, s, v)
    return self.rgb(r*255, g*255, b*255)

  def transparent(self):
    return self._register('transparent')

  def serialize(self):
    buf = np.zeros(3 * len(self.idx2color), dtype=np.uint8)
    for idx,color in enumerate(self.idx2color):
      if color == 'transparent':
        color = (255, 255, 255) # any values
      buf[idx*3+0] = color[0]
      buf[idx*3+1] = color[1]
      buf[idx*3+2] = color[2]
    return buf.tobytes()

  def transparency(self):
    if 'transparent' in self.color2idx:
      return self.color2idx['transparent']
    else:
      return None


class Visualizer:
  def __init__(self, src, dst, cache):
    self.src = src
    self.dst = dst
    self.cache = cache

    self.palette = Palette()
    self.frames = list()
    self.durations = list()

    #cmap_future = cc.m_fire#linear_ternary_red_0_50_c52
    #cmap_past = cc.m_kgy#linear_ternary_green_0_46_c42

    #cmap_future = plt.get_cmap('Reds')
    #cmap_past = plt.get_cmap('Greens')

  def make_frame(self):
    ESZ = 5 # element size in pixels
    BW = 1 # border width in pixels
    MARGIN = 20 # margin between src and dst matrices in pixels

    mats = [self.src, self.dst]

    frame_height = 0
    for mat in mats:
      frame_height = max(frame_height, mat.data.shape[0]*(ESZ+BW)+BW)

    frame_width = 0
    x_offsets = list()
    for i,mat in enumerate(mats):
      x_offsets.append(frame_width)
      frame_width += mat.data.shape[1]*(ESZ+BW) + BW
      if i+1 != len(mats):
        frame_width += MARGIN

    def draw_matrix(mat, x_offset, frame):
      draw = PIL.ImageDraw.Draw(frame)
      for i in range(mat.data.shape[0]):
        for j in range(mat.data.shape[1]):
          y1 = (ESZ + BW) * i
          y2 = (ESZ + BW) * (i + 1)
          x1 = (ESZ + BW) * j + x_offset
          x2 = (ESZ + BW) * (j + 1) + x_offset

          state = mat.data[i, j]
          age = cache.age(mat, i, j)
          if age is None:
            coef = 1.15
          else:
            coef = np.linspace(2.55, 1.55, num=cache.cache_size, endpoint=True, dtype=np.float64)[age]

          if state == Matrix.PRESENT:
            color = self.palette.rgb(0, 0, 255)
          elif state == Matrix.FUTURE:
            color = self.palette.rgb(100*coef, 0, 0)
          elif state == Matrix.PAST:
            color = self.palette.rgb(0, 100*coef, 0)
          elif state == Matrix.PADDING:
            color = self.palette.rgb(64*coef, 64*coef, 64*coef)
          else:
            raise ValueError()

          draw.rectangle(
            [(x1, y1), (x2, y2)],
            fill=color,
            outline=self.palette.rgb(0, 0, 0),
            width=1
          )

      """
      # cache lines outline
      for i in range(CLS*NCL):
        for j in range(NCL):
          y1 = (ESZ + BW) * i
          y2 = (ESZ + BW) * (i + 1)
          x1 = (ESZ*CLS + BW*CLS) * j + x_offset
          x2 = (ESZ*CLS + BW*CLS) * (j + 1) + x_offset
          draw.rectangle([(x1, y1), (x2, y2)], fill=None, outline=C_CLINE, width=1)
      """

      """
      # cache blocks outline
      for i in range(NCL):
        for j in range(NCL):
          y1 = (ESZ*CLS + BW*CLS) * i
          y2 = (ESZ*CLS + BW*CLS) * (i + 1)
          x1 = (ESZ*CLS + BW*CLS) * j + x_offset
          x2 = (ESZ*CLS + BW*CLS) * (j + 1) + x_offset
          draw.rectangle([(x1, y1), (x2, y2)], fill=None, outline=palette.register(C_CLINE), width=1)
      """

    def draw_arrows(frame):
      pillow_major = int(pkg_resources.get_distribution("pillow").version.split('.')[0])
      if pillow_major < 8:
        # text anchors
        raise ValueError(pillow_major)
      draw = PIL.ImageDraw.Draw(frame)
      fnt = PIL.ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf', MARGIN)
      for x_offset in x_offsets[1:]:
        draw.text((x_offset-MARGIN//2, frame_height//2), "➡", font=fnt, fill=self.palette.rgb(0, 0, 0), anchor='mm')

    for mat,x_offset in zip(mats, x_offsets):
      if len(self.frames) == 0:
        frame = PIL.Image.new('P', (frame_width, frame_height), color=self.palette.transparent())
        draw_arrows(frame)
      else:
        frame = self.frames[-1].copy()
      draw_matrix(mat, x_offset, frame)

      self.frames.append(frame)
      if x_offset != x_offsets[-1]:
        self.durations.append(0)
      else:
        self.durations.append(50)

  def save(self, path):
    self.frames[0].save(
      path,
      append_images=self.frames[1:],
      save_all=True,
      duration=self.durations,
      loop=0,
      palette=self.palette.serialize(),
      transparency=self.palette.transparency(),
      optimize=False
    )


algo = os.environ['ALGO']

if algo == 'srcwise':
  src = Matrix(address=5234, n=22, npad=22) # align=2/8
  dst = Matrix(address=15844, n=22, npad=22) # align=4/8
  cache = Cache(cls=8, cache_size=10)
  visualizer = Visualizer(src, dst, cache)
  for i in range(src.n):
    for j in range(src.n):
      cache.touch(src, i, j)
      cache.touch(dst, j, i)
      src.data[i,j] = Matrix.PRESENT
      dst.data[j,i] = Matrix.PRESENT
      visualizer.make_frame()
      src.data[i,j] = Matrix.PAST
      dst.data[j,i] = Matrix.PAST
  visualizer.save(f'src/{algo}.gif')

elif algo == 'srcwisepad':
  src = Matrix(address=0, n=22, npad=24)
  dst = Matrix(address=4080, n=22, npad=24)
  cache = Cache(cls=8, cache_size=10)
  visualizer = Visualizer(src, dst, cache)
  for i in range(src.n):
    for j in range(src.n):
      cache.touch(src, i, j)
      cache.touch(dst, j, i)
      src.data[i,j] = Matrix.PRESENT
      dst.data[j,i] = Matrix.PRESENT
      visualizer.make_frame()
      src.data[i,j] = Matrix.PAST
      dst.data[j,i] = Matrix.PAST
  visualizer.save(f'src/{algo}.gif')

elif algo == 'dstwise':
  src = Matrix(address=0, n=24, npad=24)
  dst = Matrix(address=4096, n=24, npad=24)
  cache = Cache(cls=8, cache_size=10)
  visualizer = Visualizer(src, dst, cache)
  for i in range(src.n):
    for j in range(src.n):
      cache.touch(src, j, i)
      cache.touch(dst, i, j)
      src.data[j,i] = Matrix.PRESENT
      dst.data[i,j] = Matrix.PRESENT
      visualizer.make_frame()
      src.data[j,i] = Matrix.PAST
      dst.data[i,j] = Matrix.PAST
  visualizer.save(f'src/{algo}.gif')

elif algo == 'blocks':
  src = Matrix(address=0, n=24, npad=24)
  dst = Matrix(address=4096, n=24, npad=24)
  cache = Cache(cls=8, cache_size=10)
  visualizer = Visualizer(src, dst, cache)

  def index(rb, cb, r, c):
    return rb*cache.cls + r, cb * cache.cls + c

  for rb in range(3):
    for cb in range(3):
      for c in range(cache.cls):
        for r in range(cache.cls):
          src_i,src_j = index(rb, cb, r, c)
          dst_i,dst_j = index(cb, rb, c, r)
          cache.touch(src, src_i, src_j)
          cache.touch(dst, dst_i, dst_j)
          is_padding = src.data[src_i,src_j] == Matrix.PADDING
          src.data[src_i,src_j] = Matrix.PRESENT
          dst.data[dst_i,dst_j] = Matrix.PRESENT
          visualizer.make_frame()
          if is_padding:
            src.data[src_i,src_j] = Matrix.PADDING
            dst.data[dst_i,dst_j] = Matrix.PADDING
          else:
            src.data[src_i,src_j] = Matrix.PAST
            dst.data[dst_i,dst_j] = Matrix.PAST
  visualizer.save(f'src/{algo}.gif')

else:
  raise ValueError()


