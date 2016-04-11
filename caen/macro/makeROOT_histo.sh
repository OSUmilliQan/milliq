#!/bin/bash

if [ $# -eq 0 ]
  then
    echo
    echo "Usage: ./makeROOT_histo.sh filename"
    echo
    exit
fi

for ARG in "$@"
do
	python translateHisto.py $ARG
done
