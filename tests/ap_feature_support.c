#include <stdio.h>
#include <stdlib.h>

struct Features_ap1 {
  int b;
  const char* name;
  const char* desc;
};

// https://nto.github.io/AirPlay.html

struct Features_ap1  features[] = {
{ 0, 	"Video", 	"video supported" },
{ 1, 	"Photo", 	"photo supported" },
{ 2, 	"VideoFairPlay", 	"video protected with FairPlay DRM" },
{ 3, 	"VideoVolumeControl", 	"volume control supported for videos" },
{ 4, 	"VideoHTTPLiveStreams", 	"http live streaming supported" },
{ 5, 	"Slideshow", 	"slideshow supported" },
{ 7, 	"Screen", 	"mirroring supported" },
{ 8, 	"ScreenRotate", 	"screen rotation supported" },
{ 9, 	"Audio", 	"audio supported" },
{ 11, 	"AudioRedundant", 	"audio packet redundancy supported" },
{ 12 ,	"FPSAPv2pt5_AES_GCM", 	"FairPlay secure auth supported" },
{ 13, 	"PhotoCaching", 	"photo preloading supported" },

{ -1, NULL, NULL }
};

struct Features_ap2 {
  int b;
  const char* name;
};

static const struct Features_ap2  features2[] = {
    { 0, "SupportsAirPlayVideoV1" },
    { 1, "SupportsAirPlayPhoto" },
    { 5, "SupportsAirPlaySlideshow" },
    { 7, "SupportsAirPlayScreen" },
    { 9, "SupportsAirPlayAudio" },
    { 11, "AudioRedunant" },
    { 14, "Authentication_4" }, // FairPlay authentication
    { 15, "MetadataFeatures_ap1_0" }, // Send artwork image to receiver
    { 16, "MetadataFeatures_ap1_1" }, // Send track progress status to receiver
    { 17, "MetadataFeatures_ap1_2" }, // Send NowPlaying info via DAAP
    { 18, "AudioFormats_0" },
    { 19, "AudioFormats_1" },
    { 20, "AudioFormats_2" },
    { 21, "AudioFormats_3" },
    { 23, "Authentication_1" }, // RSA authentication (NA)
    { 26, "Authentication_8" }, // 26 || 51, MFi authentication
    { 27, "SupportsLegacyPairing" },
    { 30, "HasUnifiedAdvertiserInfo" },
    { 32, "IsCarPlay" },
    { 32, "SupportsVolume" }, // !32
    { 33, "SupportsAirPlayVideoPlayQueue" },
    { 34, "SupportsAirPlayFromCloud" }, // 34 && flags_6_SupportsAirPlayFromCloud
    { 35, "SupportsTLS_PSK" },
    { 38, "SupportsUnifiedMediaControl" },
    { 40, "SupportsBufferedAudio" }, // srcvers >= 354.54.6 && 40
    { 41, "SupportsPTP" }, // srcvers >= 366 && 41
    { 42, "SupportsScreenMultiCodec" },
    { 43, "SupportsSystemPairing" },
    { 44, "IsAPValeriaScreenSender" },
    { 46, "SupportsHKPairingAndAccessControl" },
    { 48, "SupportsCoreUtilsPairingAndEncryption" }, // 38 || 46 || 43 || 48
    { 49, "SupportsAirPlayVideoV2" },
    { 50, "MetadataFeatures_ap1_3" }, // Send NowPlaying info via bplist
    { 51, "SupportsUnifiedPairSetupAndMFi" },
    { 52, "SupportsSetPeersExtendedMessage" },
    { 54, "SupportsAPSync" },
    { 55, "SupportsWoL" }, // 55 || 56
    { 56, "SupportsWoL" }, // 55 || 56
    { 58, "SupportsHangdogRemoteControl" }, // ((isAppleTV || isAppleAudioAccessory) && 58) || (isThirdPartyTV && flags_10)
    { 59, "SupportsAudioStreamConnectionSetup" }, // 59 && !disableStreamConnectionSetup
    { 60, "SupportsAudioMediaDataControl" }, // 59 && 60 && !disableMediaDataControl
    { 61, "SupportsRFC2198Redundancy" },
    { -1, NULL }
};


int main(int argc, char* argv[])
{
    char**  p = argv + 1;
    printf("run 'aiavahi-browse -r -k _airplay._tcp' for 'features' - airplay v1 upper limit = %ld\n", 1<<13);
    while (*p)
    {
	const long  f = strtol(*p, NULL, 16);

	struct Features_ap2*  ptr = features2;
	while (ptr->b != -1) {
	    printf("%-12s  %s\t%2d %s\n", *p, (f & (1 << ptr->b)) ? "yes" : " no", ptr->b, ptr->name);
	    ++ptr;
	}
	++p;
    }
    return 0;
}
