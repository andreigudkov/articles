#!/usr/bin/env bash
set -e
set -o pipefail

readonly root=/home/gudok/DATA

#-rw-r--r-- 1 gudok gudok 244486494 Nov  3 00:36 enwiki-20191101-pages-articles19.xml-p17620548p18754723.bz2
#-rw-r--r-- 1 gudok gudok 713705613 Nov  3 00:18 ruwiki-20191101-pages-articles5.xml-p3398622p4898622.bz2

pv ${root}/ruwiki-20191101-pages-articles5.xml-p3398622p4898622.bz2 \
  | bzcat \
  | awk '/ *<page>.*/{print text; text = "";}{text = text  $0;}' \
  > ${root}/ruwiki.txt
#cat ${root}/ruwiki.txt  | grep -v 'Обсуждение' | grep -v 'Участник' |  awk 'length($0) >= 10000' > ruwiki.txt

