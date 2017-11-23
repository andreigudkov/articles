#!/usr/bin/env python

import random
import re


class Jack:
  orig = 'All work and no play makes Jack a dull boy'
  repl = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ "
  level = 0
  styles = []
  stidx = 0

  def __init__(self):
    colors= [
      "aqua", "black", "blue", "fuchsia", "gray", "green", "lime", "maroon",
      "navy", "olive", "purple", "red", "silver", "teal", "white", "yellow"
    ]
    for color in colors:
      self.styles.append(color + " underline")
    for color in colors:
      self.styles.append(color + "-background underline")
    #self.styles.append('small')
    #self.styles.append('big')
    #self.styles.append('underline')
    #self.styles.append('overline')
    #self.styles.append('line-through')
    return

  def spoil(self, src):
    mode = random.randint(0, 3)

    if mode == 0:
      # duplicate character
      i = random.randint(0, len(src)-1)
      return src[0:i+1] + src[i] + src[i+1:]

    elif mode == 1:
      # remove character
      i = random.randint(0, len(src)-1)
      return src[0:i] + src[i+1:]

    elif mode == 2:
      # replace with random character
      i = random.randint(0, len(src)-1)
      j = -1
      while j == -1 or src[i] == self.repl[j]:
        j = random.randint(0, len(self.repl)-1)
      return src[0:i] + self.repl[j] + src[i+1:]

    elif mode == 3:
      # insert random character
      i = random.randint(0, len(src))
      j = random.randint(0, len(self.repl)-1)
      return src[0:i] + self.repl[j] + src[i+1:]

  def toListItem(self, src, isNumbered):
    self.level = random.randint(1, min(self.level+1, 5))
    if isNumbered:
      return ('.' * self.level) + " " + src
    else:
      return ('*' * self.level) + " " + src


  def colorize(self, src, cnt):
    words = re.split('[ ]', src)

    indices = dict()
    for i in range(0, min(cnt, len(self.styles)-self.stidx)):
      idx = -1
      while (idx == -1) or (len(words[idx]) < 3) or (idx in indices):
        idx = random.randint(0, len(words)-1)
      indices[idx] = True

    for i in sorted(indices.keys()):
      words[i] = "[" + self.styles[self.stidx] + "]#" + words[i] + "#"
      #self.stidx = (self.stidx + 1) % len(self.styles)
      self.stidx += 1

    return ' '.join(words)


  def generateOne(self, isNumbered):
    cnt = 0
    while cnt <= 0:
      cnt = int(random.gauss(2, 0.5))
    dst = self.orig
    for i in range(0, cnt):
      dst = self.spoil(dst)
    dst = self.colorize(dst, 2)
    dst = self.toListItem(dst, isNumbered)
    return dst

  def generateAll(self):
    for i in range(0, 8):
      print(self.generateOne(False))

    self.level = 0
    for i in range(0, 8):
      print(self.generateOne(True))

jack = Jack()
jack.generateAll()

