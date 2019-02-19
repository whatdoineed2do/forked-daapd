# OwnTone (previously forked-daapd)
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

OwnTone is a Linux/FreeBSD DAAP (iTunes), MPD (Music Player Daemon) and
RSP (Roku) media server.

It supports AirPlay devices/speakers, Apple Remote (and compatibles),
MPD clients, Chromecast, network streaming, internet radio, Spotify and LastFM.

It does not support streaming video by AirPlay nor Chromecast.

DAAP stands for Digital Audio Access Protocol which is the protocol used
by iTunes and friends to share/stream media libraries over the network.

OwnTone was previously called forked-daapd, which again was a rewrite of
mt-daapd (Firefly Media Server).


## Looking for help?

Visit the [OwnTone documentation](https://owntone.github.io/owntone-server/) for
usage and set up instructions, API documentation, etc.

If you are looking for information on how to get and install OwnTone, then see
the [Installation](https://owntone.github.io/owntone-server/installation/)
instructions.
