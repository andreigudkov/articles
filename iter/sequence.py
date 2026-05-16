#!/usr/bin/env python3

class Classic:
  def __init__(self):
    self.edges = {
      0: [1,5,9],
      1: [2],
      2: [3,4],
      3: [],
      4: [],
      5: [6,7],
      6: [],
      7: [8],
      8: [],
      9: [],
      '-': []
    }
    self.iters = [0, '-', '-']
    self.buf = ''
    self.y = 15

  def get_svg(self):
    return f'<svg width="300" height="{self.y}" xmlns="http://www.w3.org/2000/svg">' + self.buf + '</svg>'

  def advance(self, i):
    if len(self.edges[self.iters[i]]) == 0:
      return None
    item = self.edges[self.iters[i]][0]
    self.edges[self.iters[i]].pop(0)
    return item

  def dump(self, indices, item=None):
    lst = list()
    self.buf += f'<text x="5" y="{self.y}" font-family="monospace" font-size="7.5pt" xml:space="preserve">'
    for lvl,it in enumerate(self.iters):
      if it is None:
        e = 'null'
      else:
        e = f'{it}:{str(self.edges[it]).replace(" ", "")}'
      minwidth = [9,7,0][lvl]
      while len(e) < minwidth:
        e += ' '
      if lvl in indices:
        fill = indices[lvl]
        fw = 'bold'
      else:
        fill = 'black'
        fw = 'normal'
      self.buf += f'<tspan fill="{fill}" font-weight="{fw}">{e} </tspan>'
    if item is not None:
      self.buf += f'<tspan fill="green" font-weight="bold">→{item} </tspan>'
    self.buf += '</text>\n'
    self.y += 12

  def read(self):
    state = 2
    while True:
      self.dump({state:'#DA9100'}, None)
      item = self.advance(state)
      if item is not None:
        if state == 2:
          self.dump({state:'#C40234'}, item)
          return item
        else:
          self.iters[state + 1] = item
          self.dump({state:'#C40234', state+1:'#C40234'})
          state = state + 1
      else:
        if state == 0:
          return None
        else:
          state = state - 1

class Topdown:
  def __init__(self):
    self.edges = {
      0: [1,5,9],
      1: [2],
      2: [3,4],
      3: [],
      4: [],
      5: [6,7],
      6: [],
      7: [8],
      8: [],
      9: [],
    }
    self.iters = [0, None, None]
    self.buf = ''
    self.y = 15

  def dump(self, indices, item=None):
    lst = list()
    self.buf += f'<text x="5" y="{self.y}" font-family="monospace" font-size="7.5pt" xml:space="preserve">'
    for lvl,it in enumerate(self.iters):
      if it is None:
        e = 'null'
      else:
        e = f'{it}:{str(self.edges[it]).replace(" ", "")}'
      minwidth = [9,7,0][lvl]
      while len(e) < minwidth:
        e += ' '
      if lvl in indices:
        fill = indices[lvl]
        fw = 'bold'
      else:
        fill = 'black'
        fw = 'normal'
      self.buf += f'<tspan fill="{fill}" font-weight="{fw}">{e} </tspan>'
    if item is not None:
      self.buf += f'<tspan fill="green" font-weight="bold">→{item} </tspan>'
    self.buf += '</text>\n'
    self.y += 12

  def get_svg(self):
    return f'<svg width="300" height="{self.y}" xmlns="http://www.w3.org/2000/svg">' + self.buf + '</svg>'

  def is_null(self, i):
    self.dump({i:'#DA9100'})
    return self.iters[i] is None

  def is_eof(self, i):
    assert self.iters[i] is not None
    return len(self.edges[self.iters[i]]) == 0

  def reset(self, i):
    assert len(self.edges[self.iters[i]]) == 0
    self.iters[i] = None
    self.dump({i:'#C40234'})

  def renew(self, i):
    # reinitialize iterator at level i by advancing iterator i-1
    assert self.iters[i] is None
    self.iters[i] = self.__extract(i-1)
    self.dump({i-1:'#C40234', i:'#C40234'})

  def extract(self, i):
    # extract item from the bottom-most iterator
    assert i == len(self.iters)-1
    item = self.__extract(i)
    self.dump({i:'#C40234'}, item)
    return item

  def __extract(self, i):
    assert len(self.edges[self.iters[i]]) > 0
    item = self.edges[self.iters[i]][0]
    self.edges[self.iters[i]].pop(0)
    return item

  def read(self):
    while True:
      if self.is_null(0):
        return None

      if self.is_null(1):
        if not self.is_eof(0): 
          self.renew(1)
        else:
          self.reset(0)
          continue

      if self.is_null(2):
        if not self.is_eof(1): 
          self.renew(2)
        else:
          self.reset(1)
          continue

      if not self.is_eof(2):
        return self.extract(2)
      else:
        self.reset(2)
        continue

data = Classic()
while True:
  item = data.read()
  if item is None:
    break
print(data.get_svg())

