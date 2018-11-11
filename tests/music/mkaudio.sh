#!/bin/bash

if [ $# != 2 ]; then
    echo "need 2 params, input file (contain audiometa to clone) and output file"
    exit 1
fi

if [ ! -r $1 ]; then
    echo "unable to read audio meta from $1"
    exit 1
fi

DURATION=$(shuf -i 2-10 -n 1 )
sox -r 44.1k -n -twav - synth ${DURATION} sin 100-300 sin 200-80 remix 1,2 channels 2 fade 0:1 ${DURATION}  0:1  | \
   ffmpeg -i $1 -i pipe: -map 1 -map_metadata 0 -map_metadata:s:a 1:s:a $2
