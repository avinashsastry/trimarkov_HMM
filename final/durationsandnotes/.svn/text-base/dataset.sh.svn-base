#!/bin/sh
cd bols
for i in `ls *.txt`
do
	mkdir ../data/${i/.txt/};
	cp $i ../data/${i/.txt/}/;
	mv $i ../$i;
	cat *.txt > ../data/${i/.txt}/dataset.txt;
	mv ../$i $i;
done