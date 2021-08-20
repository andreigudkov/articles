#!/usr/bin/env python3

import matplotlib.pyplot as plt

def nrb():
  nn = [2112, 5824, 11584, 23232, 46400, 111808]
  timings = {
    'Naive': [3.899303, 6.682791, 6.754135, 6.621231, 6.952096, 12.395272],
    'Reverse': [2.608493, 5.077400, 5.284954, 5.272360, 6.007061, 13.742855],
    'Blocks': [1.457598, 1.585439, 1.610217, 1.619763, 1.641378, 1.650548]
  }

  colors=['#004586', '#0084d1', '#83caff']

  fig,ax = plt.subplots(figsize=(12, 4))
  height = 1
  for offset, algo in enumerate(list(timings.keys())):
    yy = [-height * ((len(timings)+2)*i+offset) for i in range(len(nn))]
    xx = timings[algo]
    ax.barh(y=yy, width=xx, height=height, label=algo, zorder=3, color=colors[offset])
    for i in range(len(nn)):
      ax.text(timings[algo][i]+height/10, yy[i], f'{timings[algo][i]:.1f}', ha='left', va='center', fontsize=9)

  ax.legend()
  ax.xaxis.grid(True, which='major', linestyle='dotted', zorder=0)
  ax.set_xlabel('Cycles per element (rdtscp)')
  ax.set_ylabel('Matrix size')
  ax.set_yticks([-((len(timings)+2)*i+(len(timings)/2))*height for i in range(len(nn))])
  ax.set_yticklabels([f'${n}^2$' for n in nn])

  tmax = max([max(timings[algo]) for algo in timings.keys()])
  tmax = int(tmax*1.2)
  ax.set_xticks([i for i in range(tmax)])
  ax.set_xticklabels([str(i) for i in range(tmax)])

  plt.savefig('src/nrb.png', bbox_inches='tight', dpi=150)

nrb()

