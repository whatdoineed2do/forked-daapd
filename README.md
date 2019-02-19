 OwnTone
# whatdoineed2do/forked-daapd
from [ejurgensen/forked-daapd](https://github.com/ejurgensen/forked-daapd)

This is the [whatdoineed2do](https://github.com/whatdoineed2do/forked-daapd) fork of the [forked-daapd upstream master](https://github.com/ejurgensen/forked-daapd).

This repo contains three branches of interest:

|Branch|Base Branch|Description|
|------|-----------|-----------|
|[master]( https://github.com/whatdoineed2do/forked-daapd/tree/master)|[forked-daapd upstream master]( https://github.com/ejurgensen/forked-daapd/tree/master)|includes patches/PRs that were not accepted into the upstream;  of particular note is my [horizontal indexed navigation for the web UI](https://github.com/ejurgensen/forked-daapd/pull/598)|, composer search/display on webui and dedicated stop button for webui
|[_ray]( https://github.com/whatdoineed2do/forked-daapd/tree/_ray)|[master]( https://github.com/whatdoineed2do/forked-daapd/tree/master)|includes [PRs currently submitted for review]( https://github.com/ejurgensen/forked-daapd/pulls/whatdoineed2do).  The codebase here is used on my _production_ system|
|[_testing]( https://github.com/whatdoineed2do/forked-daapd/tree/_testing)|[master]( https://github.com/whatdoineed2do/forked-daapd/tree/master)|this will eventually become [_ray]( https://github.com/whatdoineed2do/forked-daapd/tree/_ray) branch;  it is branched off [master]( https://github.com/whatdoineed2do/forked-daapd/tree/master) and where [PRs currently submitted for review]( https://github.com/ejurgensen/forked-daapd/pulls/whatdoineed2do) are merged before going through integration testing|
|anything else||dev branches|

If you want to experiment with this repo, take the [_ray]( https://github.com/whatdoineed2do/forked-daapd/tree/_ray) branch.

---

# forked-daapd

OwnTone is a media server that lets you play audio sources such as local files,
Spotify, pipe input or internet radio to AirPlay 1 and 2 receivers, Chromecast
receivers, Roku Soundbridge, a browser or the server’s own sound system. Or you
can listen to your music via any client that supports mp3 streaming.

You control the server via a web interface, Apple Remote, an Android remote
(e.g. Retune), an MPD client, json API or DACP.

OwnTone also serves local files via the Digital Audio Access Protocol (DAAP) to
iTunes (Windows), Apple Music (macOS) and Rhythmbox (Linux), and via the Roku
Server Protocol (RSP) to Roku devices.

Runs on Linux, BSD and macOS.

OwnTone was previously called forked-daapd, which again was a rewrite of
mt-daapd (Firefly Media Server).


## Looking for help?

Visit the [OwnTone documentation](https://owntone.github.io/owntone-server/) for
usage and set up instructions, API documentation, etc.

If you are looking for information on how to get and install OwnTone, then see
the [Installation](https://owntone.github.io/owntone-server/installation/)
instructions.
