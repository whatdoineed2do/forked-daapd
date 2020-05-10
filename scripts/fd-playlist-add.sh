#!/bin/bash

usage() {
  echo "usage: $(basename $0) [ -c ] [ -s ] <playlist location>"
  echo "       -c  clears current queue, default false"
  echo "       -s  start playback, default no"

  return 0
}

FD_SVR=http://localhost:3689

CLEAR="false"
PLAYBACK="nah"

while getopts "cs" opt; do
  case ${opt} in
    c) CLEAR="true"
      ;;
    s) PLAYBACK="start"
      ;;

    *) usage; exit 1
  esac
done
shift $((OPTIND -1))

[ $# -ne 1 ] && usage && exit 1

PLAYLIST_PATH=$1
[ ! -f ${PLAYLIST_PATH} ] && echo "no such playlist: ${PLAYLIST_PATH}" && exit 1

URI=$(curl -s -X GET ${FD_SVR}/api/library/playlists | jq -r ".items[] | select (.path == \"${PLAYLIST_PATH}\") | .uri")

[ -z $URI ] && echo "no such playlist found: $PLAYLIST_PATH" && exit 1

TRACKS=$(curl -s -X POST "${FD_SVR}/api/queue/items/add?clear=${CLEAR}&playback=${PLAYBACK}&uris=${URI}" | jq -r .count)
echo "${PLAYLIST_PATH} => ${URI}  = ${TRACKS} tracks"
