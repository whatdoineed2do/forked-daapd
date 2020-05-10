#!/bin/bash

usage() {
  echo "usage: $(basename $0) [ -c ] [ -s ] -f | -p <file|playlist location>"
  echo "       -c  clears current queue, default false"
  echo "       -s  start playback, default no"

  return 0
}

FD_SVR=http://localhost:3689

CLEAR="false"
PLAYBACK="nah"
TYPE=""

while getopts "csfp" opt; do
  case ${opt} in
    c) CLEAR="true"
      ;;
    s) PLAYBACK="start"
      ;;

    p) TYPE="playlist"
      ;;
    f) TYPE="file"
      ;;

    *) usage; exit 1
  esac
done
shift $((OPTIND -1))

[ $# -ne 1 ] && usage && exit 1
[ -z $TYPE ] && usage && exit 1

FILEPLAYLIST_PATH=$1
[ ! -f ${FILEPLAYLIST_PATH} ] && echo "no such ${TYPE}: ${FILEPLAYLIST_PATH}" && exit 1

if [ "${TYPE}" == "playlist" ]; then
  URI=$(curl -s -X GET ${FD_SVR}/api/library/playlists | jq -r ".items[] | select (.path == \"${FILEPLAYLIST_PATH}\") | .uri")
elif [ "${TYPE}" == "file" ]; then
  URI=$(curl -s -X GET "${FD_SVR}/api/search?type=track&expression=path+is+%22${FILEPLAYLIST_PATH}%22" | jq -r ".tracks | .items[].uri")
fi

[ -z $URI ] && echo "no such ${TYPE} found: $FILEPLAYLIST_PATH" && exit 1

TRACKS=$(curl -s -X POST "${FD_SVR}/api/queue/items/add?clear=${CLEAR}&playback=${PLAYBACK}&uris=${URI}" | jq -r .count)
echo "${FILEPLAYLIST_PATH} => ${URI}  = ${TRACKS} tracks"
