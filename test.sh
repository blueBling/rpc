#!/bin/bash

for i in {1..1000000}; do
{
    #sleep 1;
    b=$(($i%4000))
    if [ $b -eq 0 ]; then
        ./client>>logs/aa$i.txt && echo "done!"
    else
        ./client > /dev/null
    fi
}&
done
wait
ls logs/aa*.txt|wc -l
#rm aa
