<template>
  <div class="hero is-full-height">
    <div v-if="track.id > 0" class="hero-body is-flex is-align-items-center">
      <div class="container has-text-centered" style="max-width: 500px">
	<star-rating v-model:rating="rating"
	    :star-size="17"
	    :padding="5"
	    :show-rating="false"
	    :max-rating="5"
	    :increment="1"
	    :inline="true"
	    :clearable="true"
	    :active-on-click="true"
	    @update:rating="rate_track">
	  </star-rating>
        <cover-artwork
          :artwork_url="track.artwork_url"
          :artist="track.artist"
          :album="track.album"
          class="is-clickable fd-has-shadow fd-cover-big-image"
          :class="{ 'is-masked': lyrics_visible }"
          @click="open_dialog(track)"
        />
	<p></p>
        <p class="subtitle has-text-grey is-7" v-show="track.samplerate > 0">
         {{ track.type }} | {{ track.samplerate }} Hz | {{ now_playing_channels }} | {{ track.bitrate }} Kb/s</p>
<control-slider
          v-model:value="track_progress"
          class="mt-5"
          :disabled="is_live"
          :max="track_progress_max"
          :cursor="cursor"
          @change="seek"
          @mousedown="start_dragging"
          @mouseup="end_dragging"
        />
        <div class="is-flex is-justify-content-space-between">
          <p class="subtitle is-7" v-text="track_elapsed_time" />
          <p class="subtitle is-7" v-text="track_total_time" />
        </div>
        <p class="title is-5" v-text="track.title" />
        <p class="title is-6" v-text="track.artist" />
        <p
          v-if="composer"
          class="subtitle is-6 has-text-grey has-text-weight-bold"
          v-text="composer"
        />
        <p v-if="track.album" class="subtitle is-6" v-text="track.album" />
        <p
          v-if="filepath"
          class="subtitle is-6 has-text-grey"
          v-text="filepath"
        />
      </div>
    </div>
    <div v-else class="hero-body is-flex is-align-items-center">
      <div class="container has-text-centered">
        <p class="title is-5">Your play queue is empty</p>
        <p class="subtitle">Add tracks by browsing your library</p>
      </div>
    </div>
    <modal-dialog-queue-item
      :show="show_details_modal"
      :item="selected_item"
      :np_usermark="this.usermark"
      @close_usermark="close_usermark_upd"
      @close="show_details_modal = false"
    />
  </div>
</template>

<script>
import * as types from '@/store/mutation_types'
import ControlSlider from '@/components/ControlSlider.vue'
import CoverArtwork from '@/components/CoverArtwork.vue'
import { mdiCancel } from '@mdi/js'
import StarRating from 'vue-star-rating'
import ModalDialogQueueItem from '@/components/ModalDialogQueueItem.vue'
import webapi from '@/webapi'

const INTERVAL = 1000

export default {
  name: 'PageNowPlaying',
  components: {
    StarRating,
    ControlSlider,
    CoverArtwork,
    ModalDialogQueueItem
  },

  data() {
    return {
      cursor: mdiCancel,
      INTERVAL,
      interval_id: 0,
      is_dragged: false,

      rating: 0,
      usermark: 0,

      selected_item: {},
      show_details_modal: false
    }
  },

  computed: {
    is_live() {
      return this.track.length_ms === 0
    },

    lyrics_visible() {
      return this.$store.getters.lyrics_pane
    },

    player() {
      return this.$store.state.player
    },

    now_playing_channels () {
      if (this.track.channels === 2) {
        return 'stereo'
      }
      if (this.track.channels === 1) {
        return 'mono'
      }
      return this.track.channels
    },

    track() {
      return this.$store.getters.now_playing
    },

    track_progress: {
      get() {
        return Math.floor(this.player.item_progress_ms / INTERVAL)
      },
      set(value) {
        this.player.item_progress_ms = value * INTERVAL
      }
    },

    track_progress_max() {
      return this.is_live ? 1 : Math.floor(this.track.length_ms / INTERVAL)
    },

    track_elapsed_time() {
      return this.$filters.durationInHours(this.track_progress * INTERVAL)
    },

    track_total_time() {
      return this.is_live
        ? "live"
        : this.$filters.durationInHours(this.track.length_ms)
    },

    settings_option_show_composer_now_playing() {
      return this.$store.getters.settings_option_show_composer_now_playing
    },

    settings_option_show_composer_for_genre() {
      return this.$store.getters.settings_option_show_composer_for_genre
    },

    composer() {
      if (this.settings_option_show_composer_now_playing) {
        if (
          !this.settings_option_show_composer_for_genre ||
          (this.track.genre &&
            this.settings_option_show_composer_for_genre
              .toLowerCase()
              .split(',')
              .findIndex(
                (elem) =>
                  this.track.genre.toLowerCase().indexOf(elem.trim()) >= 0
              ) >= 0)
        ) {
          return this.track.composer
        }
      }
      return null
    },

    settings_option_show_filepath_now_playing() {
      return this.$store.getters.settings_option_show_filepath_now_playing
    },

    filepath() {
      if (this.settings_option_show_filepath_now_playing) {
        return this.track.path
      }
      return null
    }
  },

  watch: {
    player() {
      if (this.interval_id > 0) {
        window.clearTimeout(this.interval_id)
        this.interval_id = 0
      }
      if (this.player.state === 'play') {
        this.interval_id = window.setInterval(this.tick, INTERVAL)
      }
      if (this.track.track_id !== undefined) {
        webapi.library_track(this.track.track_id).then((response) => {
          this.rating = response.data.rating / 20
          this.usermark = response.data.usermark
        }).catch((err) => {
          this.rating = 0
          this.usermark = 0
          if (err.response.status === 404) {
            webapi.queue_remove(this.track.id)
          }
        })
      }
    }
  },

  created() {
    webapi.player_status().then(({ data }) => {
      this.$store.commit(types.UPDATE_PLAYER_STATUS, data)
      if (this.player.state === 'play') {
        this.interval_id = window.setInterval(this.tick, INTERVAL)
      }
    })
  },

  unmounted() {
    if (this.interval_id > 0) {
      window.clearTimeout(this.interval_id)
      this.interval_id = 0
    }
  },

  methods: {
    tick() {
      if (!this.is_dragged) {
        this.track_progress += 1
      }
    },

    start_dragging() {
      this.is_dragged = true
    },

    end_dragging() {
      this.is_dragged = false
    },

    seek() {
      if (!this.is_live) {
        webapi.player_seek_to_pos(this.track_progress * INTERVAL)
      }
    },

    rate_track: function (rating) {
      if (this.is_live) {
        return;
      }

      if (rating === 0.5) {
        rating = 0
      }
      this.rating = Math.ceil(rating)
      this.player.item_rating = this.rating * 20
      webapi.library_track_set_rating(this.track.track_id, this.rating * 20)
    },

    close_usermark_upd: function (args) {
      this.usermark = args.value
      this.show_details_modal = false
    },

    open_dialog(item) {
      this.selected_item = item
      this.show_details_modal = true
    }
  }
}
</script>

<style></style>
