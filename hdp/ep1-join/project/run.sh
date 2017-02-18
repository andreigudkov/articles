#!/usr/bin/env bash
set -e
set -o pipefail

cd build
rm -f mrhints.jar
jar -cvf mrhints.jar mrhints/
mv mrhints.jar ../
cd ..

hadoop-2.6.5/bin/hdfs dfs -rmr /enriched_sessions
java \
  -cp $(readlink -f lib)/*:$(readlink -f mrhints.jar):$(readlink -f 'hadoop-2.6.5/etc/hadoop/') \
  mrhints.DenormalizeJobV2

