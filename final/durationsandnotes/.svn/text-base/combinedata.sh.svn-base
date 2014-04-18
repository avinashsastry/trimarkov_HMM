#!/bin/bash

if [ -f TheOneFile.txt ]
then rm -rf TheOneFile.txt
fi


for i in `ls data`
do
	cat ResetTree.txt data/$i/dataset.txt TestSet.txt data/$i/$i.txt >> TheOneFile.txt;
done
	