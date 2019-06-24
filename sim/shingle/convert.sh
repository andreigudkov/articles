cat ruwiki.xml  | awk '/ *<page>.*/{print text; text = "";}{text = text  $0;}' > ruwiki.txt
cat ~/Downloads/ruwiki.txt  | grep -v 'Обсуждение' | grep -v 'Участник' |  awk 'length($0) >= 10000' > ruwiki.txt
