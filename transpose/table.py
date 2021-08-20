#!/usr/bin/env python3
import re
import os

algos = ['Naive', 'Reverse', 'Blocks', 'BlocksPrf', 'Vec32', 'Vec64', 'Vec256', 'Vec256Buf']
nn = list()
results = dict()

for name in os.listdir('results'):
  with open(f'results/{name}', 'r') as fd:
    lines = fd.readlines()
  for line in lines:
    tokens = re.split(' +', line.strip())
    algo, n, cpe, _ = tokens
    n = int(n)
    cpe = float(cpe)
    if n not in nn:
      nn.append(n)
    results[(algo,n)] = cpe

nn.sort()

print('|N|' + '|'.join(algos))
for n in nn:
  print(f'|*{n}*', end='')
  for algo in algos:
    res = results[(algo, n)]
    print(f'|{res:.2f}', end='')
  print()


