#!/usr/bin/env python3

from collections import defaultdict
import matplotlib.pyplot as plt
import matplotlib
import numpy as np

# make "i" curved
# supported values are ['dejavusans', 'dejavuserif', 'cm', 'stix', 'stixsans', 'custom']
matplotlib.rcParams.update({
  'mathtext.fontset': 'dejavuserif'
})

cmap = plt.get_cmap('tab20')

def check(base, stride, ax, order):
  nr_ways = 8
  nr_sets = 64
  set2freq = defaultdict(int)
  for i in range(stride):
    address = base + i * stride
    cache_line_index = address // 64
    cache_set = cache_line_index % nr_sets
    set2freq[cache_set] += 1

  xx = list() # x coordinate
  hh = list() # bar height
  cc = list() # classification
  bb = list() # bottom y coordinate
  for set_id in range(nr_sets):
    n = max(nr_ways, set2freq[set_id])
    for i in range(n):
      xx.append(set_id)
      hh.append(1)
      bb.append(i)
      if i < nr_ways:
        if i < set2freq[set_id]:
          cc.append('occupied')
        else:
          cc.append('free slot')
      else:
        cc.append('overload')
  xx = np.array(xx)
  hh = np.array(hh)
  cc = np.array(cc)
  bb = np.array(bb)
  colors = {
    'occupied': cmap(0),
    'free slot': cmap(1),
    'overload': cmap(6)
  }
  for classification in ['free slot', 'occupied', 'overload']:
    ii = np.where(cc == classification)
    color = colors[classification]
    if np.count_nonzero(ii) > 0:
      ax.bar(x=xx[ii], height=hh[ii], bottom=bb[ii], width=1, color=color, edgecolor='#444444', linewidth=0.5, align='edge', zorder=3, label=classification)
    else:
      ax.bar(x=[0.0], height=[0.0], bottom=[0.0], width=0, color=color, linewidth=0, align='edge', zorder=3, label=classification)

  ax.tick_params(axis='y', which='major', labelsize=12)
  if order == 0:
    ax.text(-6.0, 6.0, 'Load', ha='center', va='center', fontsize=12, rotation=90)

  ax.set_xlim([0, nr_sets])
  ax.set_xticks([0.5, 63.5])
  ax.set_xticklabels(['0', '63'])
  ax.tick_params(axis='x', which='major', labelsize=12)
  if order == 1:
    ax.text(32.0, -0.9, 'Set index', ha='center', va='center', fontsize=12)

  if order == 2:
    ax.legend()

  ax.set_title(f'${base}_{{10}}+\\mathbf{{{stride}}}i,\\;\\;i = 0..63$', loc='left')
  if np.count_nonzero(np.where(cc == 'overload')) > 0:
    ax.text(64, 16.8, '✗', ha='right', va='bottom', fontsize=16, fontweight='bold', color='red')
  else:
    ax.text(64, 16.7, '✓', ha='right', va='bottom', fontsize=18, fontweight='bold', color='green')


fig, axes = plt.subplots(
  ncols=3,
  sharey=True,
  gridspec_kw=dict(width_ratios=[1,1,1]),
  subplot_kw=dict(frameon=True),
  figsize=(12,4)
)
check(12345678, 256, axes[0], 0)
check(12345678, 293, axes[1], 1)
check(12345678, 256+64, axes[2], 2)
plt.subplots_adjust(wspace=0.05, hspace=0)
plt.savefig('src/cache_set_load.png', bbox_inches='tight', dpi=150)

