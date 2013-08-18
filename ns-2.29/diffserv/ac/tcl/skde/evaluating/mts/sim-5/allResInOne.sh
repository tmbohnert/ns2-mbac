#!/bin/sh

fn="allResultsInOne.txt"

for i in $(ls -xd voip*.txt);
do
	echo "% $i" >> $fn
	cat $i >> $fn;
done

#EOF