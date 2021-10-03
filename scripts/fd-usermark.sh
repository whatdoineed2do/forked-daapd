#!/bin/bash

TGRT=/media/public/_delete
readarray -t DIRT < <(curl -s "http://localhost:3689/api/search?type=tracks&expression=usermark+>+0" | jq -r ".tracks.items[] | select(.time_modified | fromdateiso8601 < $(date +%s --date="5days ago")) | .path" )
readarray -t DIRT1 < <(curl -s "http://rpi1:3689/api/search?type=tracks&expression=usermark+>+0" | jq -r ".tracks.items[] | select(.time_modified | fromdateiso8601 < $(date +%s --date="2days ago")) | .path" )
DIRT+=("${DIRT1[@]}")

N=${#DIRT[@]}
if [ $N -eq 0 ]; then
    echo "no data"
    exit 0
fi

for ((i=0; i<${N}; i++)); do
    FILE=${DIRT[$i]}
    [ ! -f "$FILE" ] && continue

    echo "moving $FILE"
    sudo mv "${FILE}" $TRGT/$(date +%s%N).${FILE##*.}
done
#find $TRGT -type f -mtime +30 -print -delete
exiftool '-filename<$Album - $Artist-$Title.%le' $TRGT/[0-9]*.*
