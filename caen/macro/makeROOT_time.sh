#!/bin/bash

if [ $# -eq 0 ]
  then
    echo
    echo "Usage: ./makeROOT_time.sh filename"
    echo
    exit
fi

sed -n '/=/!p' $1 > $1.tmp
python translateTime.py $1.tmp
rm $1.tmp
