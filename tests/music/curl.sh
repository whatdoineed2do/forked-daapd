#!/bin/bash

# 64bit
curl -s http://localhost:3689/api/library/artists/1125938474926605440,2284445307090104385 | jq
curl -s http://localhost:3689/api/library/artists/1125938474926605440,2284445307090104385/albums | jq
curl -s http://localhost:3689/api/library/artists/1125938474926605440,2284445307090104385/tracks| jq
curl -s http://localhost:3689/api/library/albums/1752720655991500595 | jq


# 32bit
echo "multi artist search: foo,bar"
curl -s http://localhost:3689/api/library/artists/8897714372139739445,4401536315706768408 | jq
curl -s http://localhost:3689/api/library/artists/8897714372139739445,4401536315706768408/albums | jq
curl -s http://localhost:3689/api/library/artists/8897714372139739445,4401536315706768408/tracks| jq
echo "search: bar"
curl -s "http://localhost:3689/api/search?type=track&expression=songartistid+is+%228897714372139739445%22+and+media_kind+is+music+order+by+play_count+desc&limit=10" | jq
echo "search: bar,foo"
curl -s "http://localhost:3689/api/search?type=track&expression=songartistid+in+%228897714372139739445,4401536315706768408%22+and+media_kind+is+music+order+by+play_count+desc&limit=10" | jq

echo "album foo, 1 artist"
curl -s http://localhost:3689/api/library/albums/4973605953000000699 | jq

echo "album foo/bar, multiple artists"
curl -s http://localhost:3689/api/library/albums/2001271839697385235 | jq

echo "genre"
curl -s "http://localhost:3689/api/search?type=album&expression=genre+is+\"Pop\"" | jq
echo "genre, with &"
curl -s 'http://localhost:3689/api/search?type=albums&media_kind=music&expression=genre+is+%22Drum+%26+Bass%22' | jq
echo "genre, with '"
curl -s "http://localhost:3689/api/search?type=album&expression=genre+is+\"%2790s%20alternative\"" | jq

echo "compsosers, classical'"
curl -s http://localhost:3689/api/library/composers?genre=Classical | jq
echo "tracks, classical'"
curl -s 'http://localhost:3689/api/search?type=tracks&media_kind=music&expression=genre+is+%22Classical%22' | jq
echo "artists, classical'"
curl -s 'http://localhost:3689/api/search?type=artist&media_kind=music&expression=genre+is+%22Classical%22' | jq

curl -X POST "http://localhost:3689/api/queue/items/add?expre\"/export/public/music/dnb-rs-acjbu.mp3\""
