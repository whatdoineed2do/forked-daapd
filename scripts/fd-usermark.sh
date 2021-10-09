#!/bin/bash

TRGT=/media/public/_delete


# sync other server to main

ALT_DATA=$(curl -s "http://rpi1:3689/api/search?type=tracks&expression=usermark+>+0" | jq -r ".tracks.items[] | select(.time_modified | fromdateiso8601 > $(date +%s --date="2days ago")) | {id, path, usermark}")
readarray -t ALT_PATHS < <(echo $ALT_DATA | jq -r '.path')

N=${#ALT_PATHS[@]}
for ((i=0; i<${N}; i++)); do
    TRACK_PATH=${ALT_PATHS[$i]}
    TRACK_ID=$(curl -s -G --data-urlencode 'type=tracks' --data-urlencode "expression=path is \"${TRACK_PATH}\"" http://localhost:3689/api/search | jq -r '.tracks.items[0].id')
    [ "$TRACK_ID" == "null" ] && continue

    ALT_DATA=$(echo $ALT_DATA | jq --arg argpath "$TRACK_PATH" --arg argid $TRACK_ID 'if (.path == $argpath) then (.id=$argid) else . end')
done
curl -X PUT -d "$(echo $ALT_DATA | jq -s '{tracks: . }')" http://localhost:3689/api/library/tracks


readarray -t DIRT < <(curl -s "http://localhost:3689/api/search?type=tracks&expression=usermark+>+0" | jq -r ".tracks.items[] | select(.time_modified | fromdateiso8601 < $(date +%s --date="5days ago")) | .path" )

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
