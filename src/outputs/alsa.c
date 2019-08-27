/*
 * Copyright (C) 2015-2019 Espen Jürgensen <espenjurgensen@gmail.com>
 * Copyright (C) 2010 Julien BLACHE <jb@jblache.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>

#include <alsa/asoundlib.h>

#include "misc.h"
#include "conffile.h"
#include "logger.h"
#include "player.h"
#include "outputs.h"

// We measure latency each second, and after a number of measurements determined
// by adjust_period_seconds we try to determine drift and latency. If both are
// below the two thresholds set by the below, we don't do anything. Otherwise we
// may attempt compensation by resampling. Latency is measured in samples, and
// drift is change of latency per second. Both are floats.
#define ALSA_MAX_LATENCY 480.0
#define ALSA_MAX_DRIFT 16.0
// If latency is jumping up and down we don't do compensation since we probably
// wouldn't do a good job. We use linear regression to determine the trend, but
// if r2 is below this value we won't attempt to correct sync.
#define ALSA_MAX_VARIANCE 0.3

// We correct latency by adjusting the sample rate in steps. However, if the
// latency keeps drifting we give up after reaching this step.
#define ALSA_RESAMPLE_STEP_MAX 8
// The sample rate gets adjusted by a multiple of this number. The number of
// multiples depends on the sample rate, i.e. a low sample rate may get stepped
// by 16, while high one would get stepped by 4 x 16
#define ALSA_RESAMPLE_STEP_MULTIPLE 2

#define ALSA_ERROR_WRITE -1
#define ALSA_ERROR_UNDERRUN -2
#define ALSA_ERROR_SESSION -3
#define ALSA_ERROR_DEVICE -4
#define ALSA_ERROR_DEVICE_BUSY -5

enum alsa_sync_state
{
  ALSA_SYNC_OK,
  ALSA_SYNC_AHEAD,
  ALSA_SYNC_BEHIND,
};

struct alsa_mixer
{
  snd_mixer_t *hdl;
  snd_mixer_elem_t *vol_elem;

  long vol_min;
  long vol_max;
};

struct alsa_playback_session
{
  snd_pcm_t *pcm;

  int buffer_nsamp;

  uint32_t pos;

  uint32_t last_pos;
  uint32_t last_buflen;

  struct media_quality quality;
  struct timespec last_pts;

  // Used for syncing with the clock
  struct timespec stamp_pts;
  uint64_t stamp_pos;

  // Array of latency calculations, where latency_counter tells how many are
  // currently in the array
  double *latency_history;
  int latency_counter;

  int sync_resample_step;

  // Here we buffer samples during startup
  struct ringbuffer prebuf;

  struct alsa_playback_session *next;
};

struct alsa_session
{
  enum output_device_state state;

  uint64_t device_id;
  int callback_id;

  const char *devname;
  const char *card_name;
  const char *mixer_name;
  const char *mixer_device_name;

  struct alsa_mixer mixer;

  int offset_ms;

  // A session will have multiple playback sessions when the quality changes
  struct alsa_playback_session *pb;

  struct alsa_session *next;
};

static struct alsa_session *sessions;

static bool alsa_sync_disable;
static int alsa_latency_history_size;

// We will try to play the music with the source quality, but if the card
// doesn't support that we resample to the fallback quality
static struct media_quality alsa_fallback_quality = { 44100, 16, 2, 0 };
static struct media_quality alsa_last_quality;


/* -------------------------------- FORWARDS -------------------------------- */

static void
alsa_status(struct alsa_session *as);


/* ------------------------------- MISC HELPERS ----------------------------- */

static void
dump_config(snd_pcm_t *pcm)
{
  snd_output_t *output;
  char *debug_pcm_cfg;
  int ret;

  // Dump PCM config data for E_DBG logging
  ret = snd_output_buffer_open(&output);
  if (ret == 0)
    {
      if (snd_pcm_dump_setup(pcm, output) == 0)
	{
	  snd_output_buffer_string(output, &debug_pcm_cfg);
	  DPRINTF(E_DBG, L_LAUDIO, "Dump of sound device config:\n%s\n", debug_pcm_cfg);
	}

      snd_output_close(output);
    }
}

static snd_pcm_format_t
bps2format(int bits_per_sample)
{
  if (bits_per_sample == 16)
    return SND_PCM_FORMAT_S16_LE;
  else if (bits_per_sample == 24)
    return SND_PCM_FORMAT_S24_LE;
  else if (bits_per_sample == 32)
    return SND_PCM_FORMAT_S32_LE;
  else
    return SND_PCM_FORMAT_UNKNOWN;
}

static int
volume_set(struct alsa_mixer *mixer, int volume)
{
  int pcm_vol;

  snd_mixer_handle_events(mixer->hdl);

  if (!snd_mixer_selem_is_active(mixer->vol_elem))
    return -1;

  switch (volume)
    {
      case 0:
	pcm_vol = mixer->vol_min;
	break;

      case 100:
	pcm_vol = mixer->vol_max;
	break;

      default:
	pcm_vol = mixer->vol_min + (volume * (mixer->vol_max - mixer->vol_min)) / 100;
	break;
    }

  DPRINTF(E_DBG, L_LAUDIO, "Setting ALSA volume to %d (%d)\n", pcm_vol, volume);

  snd_mixer_selem_set_playback_volume_all(mixer->vol_elem, pcm_vol);

  return 0;
}

static int
mixer_open(struct alsa_mixer *mixer, const char *mixer_device_name, const char *mixer_name)
{
  snd_mixer_t *mixer_hdl;
  snd_mixer_elem_t *vol_elem;
  snd_mixer_elem_t *elem;
  snd_mixer_elem_t *master;
  snd_mixer_elem_t *pcm;
  snd_mixer_elem_t *custom;
  snd_mixer_selem_id_t *sid;
  long vol_min;
  long vol_max;
  int ret;

  ret = snd_mixer_open(&mixer_hdl, 0);
  if (ret < 0)
    {
      DPRINTF(E_LOG, L_LAUDIO, "Failed to open mixer: %s\n", snd_strerror(ret));
      return -1;
    }

  ret = snd_mixer_attach(mixer_hdl, mixer_device_name);
  if (ret < 0)
    {
      DPRINTF(E_LOG, L_LAUDIO, "Failed to attach mixer '%s': %s\n", mixer_device_name, snd_strerror(ret));
      goto out_close;
    }

  ret = snd_mixer_selem_register(mixer_hdl, NULL, NULL);
  if (ret < 0)
    {
      DPRINTF(E_LOG, L_LAUDIO, "Failed to register mixer '%s': %s\n", mixer_device_name, snd_strerror(ret));
      goto out_detach;
    }

  ret = snd_mixer_load(mixer_hdl);
  if (ret < 0)
    {
      DPRINTF(E_LOG, L_LAUDIO, "Failed to load mixer '%s': %s\n", mixer_device_name, snd_strerror(ret));
      goto out_detach;
    }

  // Grab interesting elements
  snd_mixer_selem_id_alloca(&sid);

  pcm = NULL;
  master = NULL;
  custom = NULL;
  for (elem = snd_mixer_first_elem(mixer_hdl); elem; elem = snd_mixer_elem_next(elem))
    {
      snd_mixer_selem_get_id(elem, sid);

      if (mixer_name && (strcmp(snd_mixer_selem_id_get_name(sid), mixer_name) == 0))
	{
	  custom = elem;
	  break;
	}
      else if (strcmp(snd_mixer_selem_id_get_name(sid), "PCM") == 0)
        pcm = elem;
      else if (strcmp(snd_mixer_selem_id_get_name(sid), "Master") == 0)
	master = elem;
    }

  if (mixer_name)
    {
      if (custom)
	vol_elem = custom;
      else
	{
	  DPRINTF(E_LOG, L_LAUDIO, "Failed to open configured mixer element '%s'\n", mixer_name);
	  goto out_detach;
	}
    }
  else if (pcm)
    vol_elem = pcm;
  else if (master)
    vol_elem = master;
  else
    {
      DPRINTF(E_LOG, L_LAUDIO, "Failed to open PCM or Master mixer element\n");
      goto out_detach;
    }

  // Get min & max volume
  snd_mixer_selem_get_playback_volume_range(vol_elem, &vol_min, &vol_max);

  // All done, export
  mixer->hdl = mixer_hdl;
  mixer->vol_elem = vol_elem;
  mixer->vol_min = vol_min;
  mixer->vol_max = vol_max;

  return 0;

 out_detach:
  snd_mixer_detach(mixer_hdl, mixer_device_name);
 out_close:
  snd_mixer_close(mixer_hdl);

  return -1;
}

static void
mixer_close(struct alsa_mixer *mixer, const char *mixer_device_name)
{
  if (!mixer || !mixer->hdl)
    return;

  snd_mixer_detach(mixer->hdl, mixer_device_name);
  snd_mixer_close(mixer->hdl);
}

static int
pcm_open(snd_pcm_t **pcm, const char *device_name, struct media_quality *quality)
{
  snd_pcm_t *hdl;
  snd_pcm_hw_params_t *hw_params;
  snd_pcm_uframes_t bufsize;
  int ret;

  ret = snd_pcm_open(&hdl, device_name, SND_PCM_STREAM_PLAYBACK, 0);
  if (ret < 0)
    {
      if (ret == -EBUSY)
	return ALSA_ERROR_DEVICE_BUSY;

      DPRINTF(E_LOG, L_LAUDIO, "Could not open playback device '%s': %s\n", device_name, snd_strerror(ret));
      return ALSA_ERROR_DEVICE;
    }

  // HW params
  ret = snd_pcm_hw_params_malloc(&hw_params);
  if (ret < 0)
    {
      DPRINTF(E_LOG, L_LAUDIO, "Could not allocate hw params: %s\n", snd_strerror(ret));
      goto out_fail;
    }

  ret = snd_pcm_hw_params_any(hdl, hw_params);
  if (ret < 0)
    {
      DPRINTF(E_LOG, L_LAUDIO, "Could not retrieve hw params: %s\n", snd_strerror(ret));
      goto out_fail;
    }

  ret = snd_pcm_hw_params_set_access(hdl, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
  if (ret < 0)
    {
      DPRINTF(E_LOG, L_LAUDIO, "Could not set access method: %s\n", snd_strerror(ret));
      goto out_fail;
    }

  ret = snd_pcm_hw_params_set_format(hdl, hw_params, bps2format(quality->bits_per_sample));
  if (ret < 0)
    {
      DPRINTF(E_LOG, L_LAUDIO, "Could not set format (bits per sample %d): %s\n", quality->bits_per_sample, snd_strerror(ret));
      goto out_fail;
    }

  ret = snd_pcm_hw_params_set_channels(hdl, hw_params, quality->channels);
  if (ret < 0)
    {
      DPRINTF(E_LOG, L_LAUDIO, "Could not set stereo output: %s\n", snd_strerror(ret));
      goto out_fail;
    }

  ret = snd_pcm_hw_params_set_rate(hdl, hw_params, quality->sample_rate, 0);
  if (ret < 0)
    {
      DPRINTF(E_LOG, L_LAUDIO, "Hardware doesn't support %u Hz: %s\n", quality->sample_rate, snd_strerror(ret));
      goto out_fail;
    }

  ret = snd_pcm_hw_params_get_buffer_size_max(hw_params, &bufsize);
  if (ret < 0)
    {
      DPRINTF(E_LOG, L_LAUDIO, "Could not get max buffer size: %s\n", snd_strerror(ret));
      goto out_fail;
    }

  // Enable this line to simulate devices with low buffer size
  //bufsize = 32768;

  ret = snd_pcm_hw_params_set_buffer_size_max(hdl, hw_params, &bufsize);
  if (ret < 0)
    {
      DPRINTF(E_LOG, L_LAUDIO, "Could not set buffer size to max: %s\n", snd_strerror(ret));
      goto out_fail;
    }

  ret = snd_pcm_hw_params(hdl, hw_params);
  if (ret < 0)
    {
      DPRINTF(E_LOG, L_LAUDIO, "Could not set hw params in pcm_open(): %s\n", snd_strerror(ret));
      goto out_fail;
    }

  snd_pcm_hw_params_free(hw_params);

  *pcm = hdl;

  return 0;

 out_fail:
  snd_pcm_hw_params_free(hw_params);
  snd_pcm_close(hdl);

  return ALSA_ERROR_DEVICE;
}

static void
pcm_close(snd_pcm_t *hdl)
{
  if (!hdl)
    return;

  snd_pcm_close(hdl);
}

static int
pcm_configure(snd_pcm_t *hdl)
{
  snd_pcm_sw_params_t *sw_params;
  int ret;

  ret = snd_pcm_sw_params_malloc(&sw_params);
  if (ret < 0)
    {
      DPRINTF(E_LOG, L_LAUDIO, "Could not allocate sw params: %s\n", snd_strerror(ret));
      goto out_fail;
    }

  ret = snd_pcm_sw_params_current(hdl, sw_params);
  if (ret < 0)
    {
      DPRINTF(E_LOG, L_LAUDIO, "Could not retrieve current sw params: %s\n", snd_strerror(ret));
      goto out_fail;
    }

  ret = snd_pcm_sw_params_set_tstamp_type(hdl, sw_params, SND_PCM_TSTAMP_TYPE_MONOTONIC);
  if (ret < 0)
    {
      DPRINTF(E_LOG, L_LAUDIO, "Could not set tstamp type: %s\n", snd_strerror(ret));
      goto out_fail;
    }

  ret = snd_pcm_sw_params_set_tstamp_mode(hdl, sw_params, SND_PCM_TSTAMP_ENABLE);
  if (ret < 0)
    {
      DPRINTF(E_LOG, L_LAUDIO, "Could not set tstamp mode: %s\n", snd_strerror(ret));
      goto out_fail;
    }

  ret = snd_pcm_sw_params(hdl, sw_params);
  if (ret < 0)
    {
      DPRINTF(E_LOG, L_LAUDIO, "Could not set sw params: %s\n", snd_strerror(ret));
      goto out_fail;
    }

  snd_pcm_sw_params_free(sw_params);

  return 0;

 out_fail:
  snd_pcm_sw_params_free(sw_params);

  return -1;
}

static void
playback_session_free(struct alsa_playback_session *pb)
{
  if (!pb)
    return;

  pcm_close(pb->pcm);

  ringbuffer_free(&pb->prebuf, 1);

  free(pb->latency_history);
  free(pb);
}

static void
playback_session_remove(struct alsa_session *as, struct alsa_playback_session *pb)
{
  struct alsa_playback_session *s;

  DPRINTF(E_DBG, L_LAUDIO, "Removing playback session (quality %d/%d/%d) from ALSA device '%s'\n",
    pb->quality.sample_rate, pb->quality.bits_per_sample, pb->quality.channels, as->devname);

  if (pb == as->pb)
    as->pb = as->pb->next;
  else
    {
      for (s = as->pb; s && (s->next != pb); s = s->next)
	; /* EMPTY */

      if (!s)
	DPRINTF(E_WARN, L_LAUDIO, "WARNING: struct alsa_playback_session not found in list; BUG!\n");
      else
	s->next = pb->next;
    }

  playback_session_free(pb);
}

static void
playback_session_remove_all(struct alsa_session *as)
{
  struct alsa_playback_session *s;

  for (s = as->pb; s; s = as->pb)
    {
      as->pb = s->next;
      playback_session_free(s);
    }
}

static int
playback_session_add(struct alsa_session *as, struct media_quality *quality, struct timespec pts)
{
  struct alsa_playback_session *pb;
  struct alsa_playback_session *tail_pb;
  struct timespec ts;
  snd_pcm_sframes_t offset_nsamp;
  size_t size;
  int ret;

  DPRINTF(E_DBG, L_LAUDIO, "Adding playback session (quality %d/%d/%d) to ALSA device '%s'\n",
    quality->sample_rate, quality->bits_per_sample, quality->channels, as->devname);

  CHECK_NULL(L_LAUDIO, pb = calloc(1, sizeof(struct alsa_playback_session)));
  CHECK_NULL(L_LAUDIO, pb->latency_history = calloc(alsa_latency_history_size, sizeof(double)));

  ret = pcm_open(&pb->pcm, as->devname, quality);
  if (ret == ALSA_ERROR_DEVICE_BUSY)
    {
      DPRINTF(E_LOG, L_LAUDIO, "ALSA device '%s' won't open due to existing session (no support for concurrent audio), truncating audio\n", as->devname);
      playback_session_remove_all(as);
      ret = pcm_open(&pb->pcm, as->devname, quality);
      if (ret == ALSA_ERROR_DEVICE_BUSY)
	{
	  DPRINTF(E_LOG, L_LAUDIO, "ALSA device '%s' failed: Device still busy after closing previous sessions\n", as->devname);
	  goto error;
	}
    }

  if (ret < 0)
    {
      DPRINTF(E_LOG, L_LAUDIO, "Device '%s' does not support quality (%d/%d/%d), falling back to default\n", as->devname, quality->sample_rate, quality->bits_per_sample, quality->channels);
      ret = pcm_open(&pb->pcm, as->devname, &alsa_fallback_quality);
      if (ret < 0)
	{
	  DPRINTF(E_LOG, L_LAUDIO, "ALSA device failed setting fallback quality\n");
	  goto error;
	}

      pb->quality = alsa_fallback_quality;
    }
  else
    pb->quality = *quality;

  // If this fails it just means we won't get timestamps, which we can handle
  pcm_configure(pb->pcm);

  dump_config(pb->pcm);

  // Time stamps used for syncing, here we set when playback should start
  ts.tv_sec = OUTPUTS_BUFFER_DURATION;
  ts.tv_nsec = (uint64_t)as->offset_ms * 1000000UL;
  pb->stamp_pts = timespec_add(pts, ts);

  // The difference between pos and start pos should match the 2 second buffer
  // that AirPlay uses (OUTPUTS_BUFFER_DURATION) + user configured offset_ms. We
  // will not use alsa's buffer for the initial buffering, because my sound
  // card's start_threshold is not to be counted on. Instead we allocate our own
  // buffer, and when it is time to play we write as much as we can to alsa's
  // buffer.
  offset_nsamp = (as->offset_ms * pb->quality.sample_rate / 1000);

  pb->buffer_nsamp = OUTPUTS_BUFFER_DURATION * pb->quality.sample_rate + offset_nsamp;
  size = STOB(pb->buffer_nsamp, pb->quality.bits_per_sample, pb->quality.channels);
  ringbuffer_init(&pb->prebuf, size);

  // Add to the end of the list, because when we iterate through it in
  // alsa_write() we want to write data from the oldest playback session first
  if (as->pb)
    {
      for (tail_pb = as->pb; tail_pb->next; tail_pb = tail_pb->next)
        ; // Fast forward
      tail_pb->next = pb;
    }
  else
    as->pb = pb;

  return 0;

 error:
  playback_session_free(pb);

  return -1;
}

// This function writes the sample buf into either the prebuffer or directly to
// ALSA, depending on how much room there is in ALSA, and whether we are
// prebuffering or not. It also transfers from the the prebuffer to ALSA, if
// needed. Returns 0 on success, negative on error.
static int
buffer_write(struct alsa_playback_session *pb, struct output_data *odata, snd_pcm_sframes_t avail)
{
  uint8_t *buf;
  size_t bufsize;
  size_t wrote;
  snd_pcm_sframes_t nsamp;
  snd_pcm_sframes_t ret;

  // Prebuffering, no actual writing
  if (avail == 0)
    {
      wrote = ringbuffer_write(&pb->prebuf, odata->buffer, odata->bufsize);
      if (wrote < odata->bufsize)
	DPRINTF(E_WARN, L_LAUDIO, "Bug! Partial prebuf write %zu/%zu\n", wrote, odata->bufsize);

      nsamp = BTOS(wrote, pb->quality.bits_per_sample, pb->quality.channels);
      return nsamp;
    }

  // Read from prebuffer if it has data and write to device
  if (pb->prebuf.read_avail != 0)
    {
      // Maximum amount of bytes we want to read
      bufsize = STOB(avail, pb->quality.bits_per_sample, pb->quality.channels);

      bufsize = ringbuffer_read(&buf, bufsize, &pb->prebuf);
      if (bufsize == 0)
	return 0;

//      DPRINTF(E_DBG, L_LAUDIO, "Writing prebuffer (read_avail=%zu, bufsize=%zu, avail=%li)\n", pb->prebuf.read_avail, bufsize, avail);

      nsamp = BTOS(bufsize, pb->quality.bits_per_sample, pb->quality.channels);
      ret = snd_pcm_writei(pb->pcm, buf, nsamp);
      if (ret < 0)
	return ret;

      avail -= ret;
    }

  // Write to prebuffer if device buffer does not have availability or if we are
  // still prebuffering. Note that if the prebuffer doesn't have enough room,
  // which can happen if avail stays low, i.e. device buffer is overrunning,
  // then the extra samples get dropped
  if (odata->samples > avail || pb->prebuf.read_avail != 0)
    {
      wrote = ringbuffer_write(&pb->prebuf, odata->buffer, odata->bufsize);
      if (wrote < odata->bufsize)
	DPRINTF(E_WARN, L_LAUDIO, "Dropped %zu bytes of audio - device is overrunning!\n", odata->bufsize - wrote);

      return odata->samples;
    }

  ret = snd_pcm_writei(pb->pcm, odata->buffer, odata->samples);
  if (ret < 0)
    return ret;

  if (ret != odata->samples)
    DPRINTF(E_WARN, L_LAUDIO, "ALSA partial write detected\n");

  return ret;
}

static enum alsa_sync_state
sync_check(double *drift, double *latency, struct alsa_playback_session *pb, snd_pcm_sframes_t delay)
{
  enum alsa_sync_state sync;
  struct timespec ts;
  int elapsed;
  uint64_t cur_pos;
  uint64_t exp_pos;
  int32_t diff;
  double r2;
  int ret;

  // Would be nice to use snd_pcm_status_get_audio_htstamp here, but it doesn't
  // seem to be supported on my computer
  clock_gettime(CLOCK_MONOTONIC, &ts);

  // Here we calculate elapsed time since last reference position (which is
  // equal to playback start time, unless we have reset due to sync correction),
  // taking into account buffer time and configuration of offset_ms. We then
  // calculate our expected position based on elapsed time, and if different
  // from where we are + what is in the buffers then ALSA is out of sync.
  elapsed = (ts.tv_sec - pb->stamp_pts.tv_sec) * 1000L + (ts.tv_nsec - pb->stamp_pts.tv_nsec) / 1000000;
  if (elapsed < 0)
    return ALSA_SYNC_OK;

  cur_pos = (uint64_t)pb->pos - pb->stamp_pos - (delay + BTOS(pb->prebuf.read_avail, pb->quality.bits_per_sample, pb->quality.channels));
  exp_pos = (uint64_t)elapsed * pb->quality.sample_rate / 1000;
  diff = cur_pos - exp_pos;

  DPRINTF(E_SPAM, L_LAUDIO, "counter %d/%d, stamp %lu:%lu, now %lu:%lu, elapsed is %d ms, cur_pos=%" PRIu64 ", exp_pos=%" PRIu64 ", diff=%d\n",
    pb->latency_counter, alsa_latency_history_size, pb->stamp_pts.tv_sec, pb->stamp_pts.tv_nsec / 1000000, ts.tv_sec, ts.tv_nsec / 1000000, elapsed, cur_pos, exp_pos, diff);

  // Add the latency to our measurement history
  pb->latency_history[pb->latency_counter] = (double)diff;
  pb->latency_counter++;

  // Haven't collected enough samples for sync evaluation yet, so just return
  if (pb->latency_counter < alsa_latency_history_size)
    return ALSA_SYNC_OK;

  pb->latency_counter = 0;

  ret = linear_regression(drift, latency, &r2, NULL, pb->latency_history, alsa_latency_history_size);
  if (ret < 0)
    {
      DPRINTF(E_WARN, L_LAUDIO, "Linear regression of collected latency samples failed\n");
      return ALSA_SYNC_OK;
    }

  // Set *latency to the "average" within the period
  *latency = (*drift) * alsa_latency_history_size / 2 + (*latency);

  if (abs(*latency) < ALSA_MAX_LATENCY && abs(*drift) < ALSA_MAX_DRIFT)
    sync = ALSA_SYNC_OK; // If both latency and drift are within thresholds -> no action
  else if (*latency > 0 && *drift > 0)
    sync = ALSA_SYNC_AHEAD;
  else if (*latency < 0 && *drift < 0)
    sync = ALSA_SYNC_BEHIND;
  else
    sync = ALSA_SYNC_OK; // Drift is counteracting latency -> no action

  if (sync != ALSA_SYNC_OK && r2 < ALSA_MAX_VARIANCE)
    {
      DPRINTF(E_DBG, L_LAUDIO, "Too much variance in latency measurements (r2=%f/%f), won't try to compensate\n", r2, ALSA_MAX_VARIANCE);
      sync = ALSA_SYNC_OK;
    }

  DPRINTF(E_DBG, L_LAUDIO, "Sync check result: drift=%f, latency=%f, r2=%f, sync=%d\n", *drift, *latency, r2, sync);

  return sync;
}

static void
sync_correct(struct alsa_playback_session *pb, double drift, double latency, struct timespec pts, snd_pcm_sframes_t delay)
{
  int step;
  int sign;

  // We change the sample_rate in steps that are a multiple of 50. So we might
  // step 44100 -> 44000 -> 40900 -> 44000 -> 44100. If we used percentages to
  // to step, we would have to deal with rounding; we don't want to step 44100
  // -> 39996 -> 44099.
  step = ALSA_RESAMPLE_STEP_MULTIPLE * (pb->quality.sample_rate / 20000);

  sign = (drift < 0) ? -1 : 1;

  if (abs(pb->sync_resample_step) == ALSA_RESAMPLE_STEP_MAX)
    {
      DPRINTF(E_LOG, L_LAUDIO, "The sync of ALSA device cannot be corrected (drift=%f, latency=%f)\n", drift, latency);
      pb->sync_resample_step += sign;
      return;
    }
  else if (abs(pb->sync_resample_step) > ALSA_RESAMPLE_STEP_MAX)
    return; // Don't do anything, we have given up

  // Step 0 is the original audio quality (or the fallback quality), which we
  // will just keep receiving
  if (pb->sync_resample_step != 0)
    outputs_quality_unsubscribe(&pb->quality);

  pb->sync_resample_step += sign;
  pb->quality.sample_rate += sign * step;

  if (pb->sync_resample_step != 0)
    outputs_quality_subscribe(&pb->quality);

  // Reset position so next sync_correct latency correction is only based on
  // what has elapsed since our correction
  pb->stamp_pos = (uint64_t)pb->pos - (delay + BTOS(pb->prebuf.read_avail, pb->quality.bits_per_sample, pb->quality.channels));;
  pb->stamp_pts = pts;

  DPRINTF(E_INFO, L_LAUDIO, "Adjusted sample rate to %d to sync ALSA device (drift=%f, latency=%f)\n", pb->quality.sample_rate, drift, latency);
}

static int
playback_drain(struct alsa_playback_session *pb)
{
  uint8_t *buf;
  size_t bufsize;
  snd_pcm_state_t state;
  snd_pcm_sframes_t avail;
  snd_pcm_sframes_t delay;
  snd_pcm_sframes_t nsamp;
  int ret;

  state = snd_pcm_state(pb->pcm);
  if (state == SND_PCM_STATE_DRAINING)
    return 0;
  else if (state != SND_PCM_STATE_RUNNING)
    return ALSA_ERROR_SESSION; // We are probably done draining, so this makes the caller close the pb session

  // If the prebuffer is empty we are done writing to this pcm
  if (pb->prebuf.read_avail == 0)
    {
      snd_pcm_drain(pb->pcm); // Plays pending frames and then stops the pcm
      return 0;
    }

  ret = snd_pcm_avail_delay(pb->pcm, &avail, &delay);
  if (ret < 0)
    {
      DPRINTF(E_LOG, L_LAUDIO, "Error getting avail/delay: %s\n", snd_strerror(ret));
      return ALSA_ERROR_SESSION;
    }

  // Maximum amount of bytes we want to read
  bufsize = STOB(avail, pb->quality.bits_per_sample, pb->quality.channels);

  bufsize = ringbuffer_read(&buf, bufsize, &pb->prebuf);
  if (bufsize == 0)
    return 0; // avail too low to actually write anything

//  DPRINTF(E_DBG, L_LAUDIO, "Draining prebuffer (read_avail=%zu, bufsize=%zu, avail=%li)\n", pb->prebuf.read_avail / 4, bufsize, avail);

  nsamp = BTOS(bufsize, pb->quality.bits_per_sample, pb->quality.channels);

  ret = snd_pcm_writei(pb->pcm, buf, nsamp);

  return ((ret < 0) ? ALSA_ERROR_SESSION : 0);
}

static int
playback_write(struct alsa_playback_session *pb, struct output_buffer *obuf)
{
  snd_pcm_sframes_t avail;
  snd_pcm_sframes_t delay;
  enum alsa_sync_state sync;
  double drift;
  double latency;
  bool prebuffering;
  int ret;
  int i;

  // Find the quality we want
  for (i = 0; obuf->data[i].buffer; i++)
    {
      if (quality_is_equal(&pb->quality, &obuf->data[i].quality))
	break;
    }

  if (!obuf->data[i].buffer)
    {
      DPRINTF(E_LOG, L_LAUDIO, "Output not delivering required data quality, aborting\n");
      return -1;
    }

  prebuffering = (pb->pos + obuf->data[i].bufsize <= pb->buffer_nsamp);
  if (prebuffering)
    {
      // Can never fail since we don't actually write to the device
      pb->pos += buffer_write(pb, &obuf->data[i], 0);
      return 0;
    }

  ret = snd_pcm_avail_delay(pb->pcm, &avail, &delay);
  if (ret < 0)
    goto alsa_error;

  // Check sync each second (or if this is first write where last_pts is zero)
  if (!alsa_sync_disable && (obuf->pts.tv_sec != pb->last_pts.tv_sec))
    {
      sync = sync_check(&drift, &latency, pb, delay);
      if (sync != ALSA_SYNC_OK)
	sync_correct(pb, drift, latency, obuf->pts, delay);

      pb->last_pts = obuf->pts;
    }

  ret = buffer_write(pb, &obuf->data[i], avail);
  if (ret < 0)
    goto alsa_error;

  pb->pos += ret;

  return 0;

 alsa_error:
  if (ret == -EPIPE)
    {
      DPRINTF(E_WARN, L_LAUDIO, "ALSA buffer underrun, restarting session\n");
      return ALSA_ERROR_UNDERRUN;
    }

  DPRINTF(E_LOG, L_LAUDIO, "ALSA write error: %s\n", snd_strerror(ret));
  return ALSA_ERROR_WRITE;
}


/* ---------------------------- SESSION HANDLING ---------------------------- */

static void
alsa_session_free(struct alsa_session *as)
{
  if (!as)
    return;

  outputs_quality_unsubscribe(&alsa_fallback_quality);

  playback_session_remove_all(as);

  mixer_close(&as->mixer, as->mixer_device_name);

  free(as);
}

static void
alsa_session_cleanup(struct alsa_session *as)
{
  struct alsa_session *s;

  if (as == sessions)
    sessions = sessions->next;
  else
    {
      for (s = sessions; s && (s->next != as); s = s->next)
	; /* EMPTY */

      if (!s)
	DPRINTF(E_WARN, L_LAUDIO, "WARNING: struct alsa_session not found in list; BUG!\n");
      else
	s->next = as->next;
    }

  outputs_device_session_remove(as->device_id);

  alsa_session_free(as);
}

static struct alsa_session *
alsa_session_make(struct output_device *device, int callback_id)
{
  struct alsa_session *as;
  cfg_t *cfg_audio;
  int ret;

  CHECK_NULL(L_LAUDIO, as = calloc(1, sizeof(struct alsa_session)));

  as->device_id = device->id;
  as->callback_id = callback_id;

  cfg_audio = cfg_getsec(cfg, "audio");

  as->devname = cfg_getstr(cfg_audio, "card");
  as->mixer_name = cfg_getstr(cfg_audio, "mixer");
  as->mixer_device_name = cfg_getstr(cfg_audio, "mixer_device");
  if (!as->mixer_device_name || strlen(as->mixer_device_name) == 0)
    as->mixer_device_name = cfg_getstr(cfg_audio, "card");

  as->offset_ms = cfg_getint(cfg_audio, "offset_ms");
  if (abs(as->offset_ms) > 1000)
    {
      DPRINTF(E_LOG, L_LAUDIO, "The ALSA offset_ms (%d) set in the configuration is out of bounds\n", as->offset_ms);
      as->offset_ms = 1000 * (as->offset_ms/abs(as->offset_ms));
    }

  ret = mixer_open(&as->mixer, as->mixer_device_name, as->mixer_name);
  if (ret < 0)
    {
      DPRINTF(E_LOG, L_LAUDIO, "Could not open mixer '%s' ('%s')\n", as->mixer_device_name, as->mixer_name);
      goto error_free_session;
    }

  ret = outputs_quality_subscribe(&alsa_fallback_quality);
  if (ret < 0)
    {
      DPRINTF(E_LOG, L_LAUDIO, "Could not subscribe to fallback audio quality\n");
      goto error_mixer_close;
    }

  as->state = OUTPUT_STATE_CONNECTED;
  as->next = sessions;
  sessions = as;

  // as is now the official device session
  outputs_device_session_add(device->id, as);

  return as;

 error_mixer_close:
  mixer_close(&as->mixer, as->mixer_device_name);
 error_free_session:
  free(as);
  return NULL;
}

static void
alsa_status(struct alsa_session *as)
{
  outputs_cb(as->callback_id, as->device_id, as->state);
  as->callback_id = -1;

  if (as->state == OUTPUT_STATE_FAILED || as->state == OUTPUT_STATE_STOPPED)
    alsa_session_cleanup(as);
}


/* ------------------ INTERFACE FUNCTIONS CALLED BY OUTPUTS.C --------------- */

static int
alsa_device_start(struct output_device *device, int callback_id)
{
  struct alsa_session *as;

  as = alsa_session_make(device, callback_id);
  if (!as)
    return -1;

  as->state = OUTPUT_STATE_CONNECTED;
  alsa_status(as);

  return 0;
}

static int
alsa_device_stop(struct output_device *device, int callback_id)
{
  struct alsa_session *as = device->session;

  as->callback_id = callback_id;
  as->state = OUTPUT_STATE_STOPPED;
  alsa_status(as); // Will terminate the session since the state is STOPPED

  return 0;
}

static int
alsa_device_flush(struct output_device *device, int callback_id)
{
  struct alsa_session *as = device->session;

  playback_session_remove_all(as);

  as->callback_id = callback_id;
  as->state = OUTPUT_STATE_CONNECTED;
  alsa_status(as);

  return 0;
}

static int
alsa_device_probe(struct output_device *device, int callback_id)
{
  struct alsa_session *as;

  as = alsa_session_make(device, callback_id);
  if (!as)
    return -1;

  as->state = OUTPUT_STATE_STOPPED;
  alsa_status(as); // Will terminate the session since the state is STOPPED

  return 0;
}

static int
alsa_device_volume_set(struct output_device *device, int callback_id)
{
  struct alsa_session *as = device->session;

  if (!as)
    return 0;

  volume_set(&as->mixer, device->volume);

  as->callback_id = callback_id;
  alsa_status(as);

  return 1;
}

static void
alsa_device_cb_set(struct output_device *device, int callback_id)
{
  struct alsa_session *as = device->session;

  as->callback_id = callback_id;
}

static void
alsa_write(struct output_buffer *obuf)
{
  struct alsa_session *as;
  struct alsa_session *as_next;
  struct alsa_playback_session *pb;
  struct alsa_playback_session *pb_next;
  bool quality_changed;
  int ret;

  quality_changed = !quality_is_equal(&obuf->data[0].quality, &alsa_last_quality);
  alsa_last_quality = obuf->data[0].quality;

  for (as = sessions; as; as = as->next)
    {
      if (quality_changed || as->state == OUTPUT_STATE_CONNECTED)
	{
	  ret = playback_session_add(as, &obuf->data[0].quality, obuf->pts);
	  if (ret < 0)
	    {
	      as->state = OUTPUT_STATE_FAILED;
	      continue;
	    }

	  as->state = OUTPUT_STATE_STREAMING;
	}

      for (pb = as->pb; pb; pb = pb_next)
	{
	  pb_next = pb->next;

	  // If !pb_next then it means that it is the most recent session, so it
	  // is setup with the quality level that matches obuf. The other pb's
	  // may still have data that needs to be written before removal.
	  if (!pb_next)
	    ret = playback_write(pb, obuf);
	  else
	    ret = playback_drain(pb);

	  if (ret < 0)
	    {
	      playback_session_remove(as, pb); // pb becomes invalid
	      if (ret == ALSA_ERROR_WRITE)
		as->state = OUTPUT_STATE_FAILED;
	      else if (ret == ALSA_ERROR_UNDERRUN)
		as->state = OUTPUT_STATE_CONNECTED;
	    }
	}
    }

  // Cleanup failed sessions
  for (as = sessions; as; as = as_next)
    {
      as_next = as->next;

      if (as->state == OUTPUT_STATE_FAILED)
	alsa_status(as); // as becomes invalid
    }
}

static int
alsa_init(void)
{
  struct output_device *device;
  cfg_t *cfg_audio;
  const char *type;

  // Is ALSA enabled in config?
  cfg_audio = cfg_getsec(cfg, "audio");
  type = cfg_getstr(cfg_audio, "type");
  if (type && (strcasecmp(type, "alsa") != 0))
    return -1;

  alsa_sync_disable = cfg_getbool(cfg_audio, "sync_disable");
  alsa_latency_history_size = cfg_getint(cfg_audio, "adjust_period_seconds");

  CHECK_NULL(L_LAUDIO, device = calloc(1, sizeof(struct output_device)));

  device->id = 0;
  device->name = strdup(cfg_getstr(cfg_audio, "nickname"));
  device->type = OUTPUT_TYPE_ALSA;
  device->type_name = outputs_name(device->type);
  device->has_video = 0;

  DPRINTF(E_INFO, L_LAUDIO, "Adding ALSA device '%s' with name '%s'\n", cfg_getstr(cfg_audio, "card"), device->name);

  player_device_add(device);

  snd_lib_error_set_handler(logger_alsa);

  return 0;
}

static void
alsa_deinit(void)
{
  struct alsa_session *as;

  snd_lib_error_set_handler(NULL);

  for (as = sessions; sessions; as = sessions)
    {
      sessions = as->next;
      alsa_session_free(as);
    }
}

struct output_definition output_alsa =
{
  .name = "ALSA",
  .type = OUTPUT_TYPE_ALSA,
  .priority = 3,
  .disabled = 0,
  .init = alsa_init,
  .deinit = alsa_deinit,
  .device_start = alsa_device_start,
  .device_stop = alsa_device_stop,
  .device_flush = alsa_device_flush,
  .device_probe = alsa_device_probe,
  .device_volume_set = alsa_device_volume_set,
  .device_cb_set = alsa_device_cb_set,
  .write = alsa_write,
};
