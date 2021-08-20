#!/usr/bin/env python3

import re
from dataclasses import dataclass
import matplotlib.pyplot as plt
import numpy as np
from matplotlib import gridspec

@dataclass
class Data:
  nn:object = None # matrix size
  pp:object = None # num passes
  tt:object = None # rdtscp per element
  l2hit:object = None # l2_rqsts.rfo_{hit|miss}
  l2miss:object = None

def normalize(data:Data):
  ii = np.argsort(data.nn)
  return Data(
    nn = np.asarray(data.nn, dtype=np.int64)[ii],
    pp = np.asarray(data.pp, dtype=np.int64)[ii],
    tt = np.asarray(data.tt, dtype=np.float64)[ii],
    l2hit = np.asarray(data.l2hit, dtype=np.float64)[ii],
    l2miss = np.asarray(data.l2miss, dtype=np.float64)[ii],
  )


def naive_transpose_experiment(data):
  data = normalize(data)

  fig, (ax1,ax0) = plt.subplots(ncols=2, sharey=True, gridspec_kw=dict(width_ratios=[3,10]), subplot_kw=dict(frameon=True), figsize=(12,6))

  # vertical position
  yy = [len(data.tt)-i-0.5 for i in range(len(data.tt))]

  # interleave bar colors
  cc = list()
  cmap = plt.get_cmap('tab20')
  for i in range(len(yy)):
    if i % 2 == 0:
      cc.append(cmap(0))
    else:
      cc.append(cmap(1))

  # plot
  ax0.barh(yy, data.tt, height=1.0, color=cc, edgecolor=cc, linewidth=0.5, zorder=3)

  # labels
  ax0.set_xlabel("Cycles per element (rdtscp)")
  ax0.set_yticks([yy[i] for i in range(len(yy)) if i%2 == 0])
  ax0.set_yticklabels([f'${data.nn[i]}^2$' for i in range(len(yy)) if i%2 == 0])
  #ax0.set_yticks([yy[i] for i in range(len(yy))])
  #ax0.set_yticklabels([f'${data.nn[i]}^2$' for i in range(len(yy))])
  ax0.tick_params(axis='y', which='major', labelsize=9)
  ax0.tick_params(axis='y', which='minor', labelsize=9)
  ax0.set_ylim([0, len(data.tt)])
  ax0.set_xlim([0, max(data.tt)*1.1])

  # text labels
  for i in range(len(data.tt)):
    localmax = True
    for j in [i-1, i+1]:
      if (j < 0) or (j >= len(data.tt)):
        continue
      if data.tt[j] > data.tt[i]-1:
        localmax = False
        break
    if localmax:
      if i == 0:
        va = 'top'
      elif i == len(data.tt)-1:
        va = 'bottom'
      else:
        va = 'center'
      ax0.text(data.tt[i]+0.2, yy[i], f'{data.tt[i]:.1f}', ha='left', va=va, fontsize=9)

  ax0.yaxis.set_tick_params(left=True, labelleft=True, right=False, labelright=False)
  ax0.xaxis.grid(True, which='major', linestyle='dotted', zorder=0)

  # description
  lines = [
    'i7-7700HQ 2.80GHz',
    '',
    'L1d:  32KiB',
    'L2:  256KiB',
    'L3: 6144KiB'
  ]
  ax0.text(max(data.tt)*1.08, max(yy)*0.97, 'Naive transpose algorithm', ha='right', va='top', fontweight='bold', fontsize=12, backgroundcolor='white')
  ax0.text(max(data.tt)*1.08, max(yy)*0.9, '\n'.join(lines), ha='right', va='top', backgroundcolor='white', fontsize=10, fontfamily='monospace')



  ax1.barh(
    yy,
    [data.l2hit[i]/(data.nn[i]*data.nn[i]*data.pp[i]) for i in range(len(data.tt))],
    height=0.8, color='#aecf00',
    zorder=2
  )
  ax1.barh(
    yy,
    [data.l2miss[i]/(data.nn[i]*data.nn[i]*data.pp[i]) for i in range(len(data.tt))],
    left=[1.0 - data.l2miss[i]/(data.nn[i]*data.nn[i]*data.pp[i]) for i in range(len(data.tt))],
    height=0.8, color='#7e6427',
    zorder=2
  )


  ax1.set_xlim([0.0, 1.0])
  ax1.set_xlabel('L2 write events per element')
  ax1.yaxis.set_tick_params(left=False, labelleft=False, right=True, labelright=False)
  ax1.xaxis.grid(True, which='major', linestyle='dotted', zorder=0)
  ax1.text(-0.05, max(yy)/2, 'Matrix size', ha='right', va='center', rotation='vertical')

  ax1.text(0.3, 48, 'l2_rqsts.rfo_hit',
    va='center', ha='center',
    rotation=30, fontweight='bold', fontsize='small',
    bbox=dict(boxstyle='round', color='white', linewidth=0, alpha=1.0))

  ax1.text(0.5, 10, 'l2_rqsts.rfo_miss',
    va='center', ha='center',
    rotation=30, fontweight='bold', fontsize='small',
    bbox=dict(boxstyle='round', color='white', linewidth=0, alpha=1.0))

  plt.savefig('src/naive_transpose_experiment.png', bbox_inches='tight', dpi=150)

data = Data(
  nn=[256,272,298,304,368,381,385,416,434,463,487,592,640,775,997,1024,1157,1184,1327,1368,1397,1641,1664,2003,2048,2097,2177,2260,2674,3349,3388,3638,4096,4726,5296,5569,5712,6828,7183,8663,9474,10560,13776,14401,17522,19069,20491,21888,22778,25622,26848,27088,30720,31968,34944,38440,39963,45383,58838,66816,77824,87456,93967,104405,108430,109644,111744,113472,114930,],
  tt=[6.070376,2.344284,2.300841,2.338413,2.328428,3.566934,2.249341,2.346107,2.858916,2.292273,2.533351,3.983479,5.246746,4.268127,4.279357,13.284690,4.196692,4.121748,4.127669,7.418052,4.181687,5.714137,5.312905,4.123522,14.011552,5.380636,4.430942,8.224077,4.343082,5.801435,5.235156,6.626007,23.405534,7.577792,7.034732,6.556128,7.247461,10.120080,6.678182,6.895115,8.515650,6.518783,8.323120,6.815290,7.650477,8.012680,7.671895,9.624374,7.777312,6.767505,8.568170,6.834996,24.339635,7.320653,9.786448,8.289127,7.310052,6.873375,8.208774,22.713839,26.857144,10.368746,11.281753,11.667679,12.377022,12.420580,21.750734,12.531617,12.983490,],
  pp=[131072,116105,96729,92948,63429,59175,57951,49636,45604,40070,36218,24510,20971,14301,8641,8192,6416,6127,4878,4590,4401,3189,3102,2141,2048,1953,1812,1681,1201,765,748,649,512,384,306,276,263,184,166,114,95,77,45,41,27,23,20,17,16,13,11,11,9,8,7,5,5,4,2,1,1,1,1,1,1,1,1,1,1,],
  l2hit=[8581374378,113313529,317656737,276426642,277509926,4438966065,202219039,380649587,2180057356,813580379,1506533099,6108373876,8492007365,7123721300,8484151881,1583689949,8458728706,8471523467,8488154827,8729920518,8501922782,8547807428,8109679702,8445605501,181077247,5518981940,8399045336,228117075,7505290370,5846897455,5833855470,4213655639,206288545,2188708978,1891400863,1307123519,722368731,3030265039,132901458,799963,757850,598149,4025970,609257,21524438,38139680,66576259,1608028,2185892,1318963,1029562,593126,1957577875,1590301,1197620,163316869,686987,1079353,951421,8738501,230121208,1481842,54386196,7303474,1234746,3469653,1298485,576651,2937337,],
  l2miss=[36686283,68102167,58379733,59536704,65538462,37733549,81603614,62512030,65220858,63990402,60785217,69663767,94704292,101996018,100648779,8594214960,126925569,114977480,99112407,127757766,85007350,150387764,479666548,147424265,8600858966,3069069699,189693249,8357328455,1083286670,2733122492,2751721847,4392972245,8704219595,6479837684,6691868092,7252473950,7860178310,8580018534,8434012885,8555288004,8528315473,8587432788,8542413091,8504088759,8291614152,8328063245,8399835444,8147854827,8303202917,8535874384,7929857931,8072924413,8525500676,8179686841,8550307882,7390529742,7987173075,8240411283,6925668114,4468318213,6189762608,7651816546,8832807413,10916694158,11758505422,12030710715,12490280927,12877336491,13210733858,]
)

naive_transpose_experiment(data)

