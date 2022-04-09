<template>
  <section>
    <div v-if="now_playing.id > 0" class="fd-is-fullheight">
      <div class="has-text-centered fd-has-padding-left-right"><star-rating v-model="rating"
        :star-size="17"
        :padding="5"
        :show-rating="false"
        :max-rating="5"
        :increment="1"
        :inline="true"
        :clearable="true"
        :active-on-click="true"
        @rating-selected="rate_track"></star-rating> </div>
      <div class="fd-is-expanded">
        <cover-artwork @click="open_dialog(now_playing)"
          :artwork_url="now_playing.artwork_url"
          :artist="now_playing.artist"
          :album="now_playing.album"
          class="fd-cover-image fd-has-action" />
      </div>
      <div class="fd-has-padding-left-right">
        <div class="container has-text-centered">
          <p class="subtitle has-text-grey is-7" v-show="now_playing.samplerate > 0">
          {{ now_playing.type }} | {{ now_playing.samplerate }} Hz | {{ now_playing_channels }} | {{ now_playing.bitrate }} Kb/s
          </p>
          <p class="control has-text-centered fd-progress-now-playing">
            <range-slider
              class="seek-slider fd-has-action"
              min="0"
              :max="state.item_length_ms"
              :value="item_progress_ms"
              :disabled="state.state === 'stop' || seeking"
              step="1000"
              @change="seek" >
            </range-slider>
          </p>
          <p class="content">
            <span>{{ item_progress_ms | duration }} / {{ now_playing.length_ms | duration }}</span>
          </p>
        </div>
      </div>
      <div class="fd-has-padding-left-right">
        <div class="container has-text-centered fd-has-margin-top">
          <h1 :class="{ 'title': true, 'is-5': true, 'has-text-grey': this.usermark > 0, 'is-italic': this.usermark > 0}">
            {{ now_playing.title }}
            <h2 class="subtitle is-7 has-text-grey" v-if="composer">
              {{ composer }}
            </h2>
          </h1>
          <h2 class="title is-6">
            {{ now_playing.artist }}
          </h2>
          <h2 class="subtitle is-6 has-text-grey has-text-weight-bold" v-if="composer">
              {{ composer }}
          </h2>
          <h3 class="subtitle is-6">
            {{ now_playing.album }}
          </h3>
        </div>
      </div>
    </div>
    <div v-else class="fd-is-fullheight">
      <div class="fd-is-expanded fd-has-padding-left-right" style="flex-direction: column;">
        <div class="content has-text-centered">
          <h1 class="title is-5">
            Your play queue is empty
          </h1>
          <p>
            Add some tracks by browsing your library
          </p>
        </div>
      </div>
    </div>
    <modal-dialog-queue-item :show="show_details_modal" :item="selected_item" :np_usermark="this.usermark" @close="show_details_modal = false" @close_usermark="close_usermark_upd"/>
  </section>
</template>

<script>
import ModalDialogQueueItem from '@/components/ModalDialogQueueItem'
import RangeSlider from 'vue-range-slider'
import CoverArtwork from '@/components/CoverArtwork'
import StarRating from 'vue-star-rating'
import webapi from '@/webapi'
import * as types from '@/store/mutation_types'

export default {
  name: 'PageNowPlaying',
  components: { ModalDialogQueueItem, RangeSlider, CoverArtwork, StarRating },

  data () {
    return {
      is_seeking: false,
      item_progress_ms: 0,
      interval_id: 0,

      rating: 0,
      usermark: 0,

      show_details_modal: false,
      selected_item: {}
    }
  },

  created () {
    this.item_progress_ms = this.state.item_progress_ms
    webapi.player_status().then(({ data }) => {
      this.$store.commit(types.UPDATE_PLAYER_STATUS, data)
      if (this.state.state === 'play') {
        this.interval_id = window.setInterval(this.tick, 1000)
      }
    })
  },

  destroyed () {
    if (this.interval_id > 0) {
      window.clearTimeout(this.interval_id)
      this.interval_id = 0
    }
  },

  computed: {
    state () {
      return this.$store.state.player
    },

    now_playing () {
      return this.$store.getters.now_playing
    },

    now_playing_channels () {
      if (this.now_playing.channels === 2) {
        return 'stereo'
      }
      if (this.now_playing.channels === 1) {
        return 'mono'
      }
      return this.now_playing.channels
    },
    seeking: function () {
      return this.is_seeking
    },

    settings_option_show_composer_now_playing () {
      return this.$store.getters.settings_option_show_composer_now_playing
    },

    settings_option_show_composer_for_genre () {
      return this.$store.getters.settings_option_show_composer_for_genre
    },

    composer () {
      if (this.settings_option_show_composer_now_playing) {
        if (!this.settings_option_show_composer_for_genre ||
            (this.now_playing.genre &&
            this.settings_option_show_composer_for_genre.toLowerCase()
              .split(',')
              .findIndex(elem => this.now_playing.genre.toLowerCase().indexOf(elem.trim()) >= 0) >= 0)) {
          return this.now_playing.composer
        }
      }
      return null
    }
  },

  methods: {
    tick: function () {
      this.item_progress_ms += 1000
    },

    seek: function (newPosition) {
      this.is_seeking = true
      webapi.player_seek_to_pos(newPosition).catch(() => {
        this.item_progress_ms = this.state.item_progress_ms
      })
      this.is_seeking = false
    },

    rate_track: function (rating) {
      if (rating === 0.5) {
        rating = 0
      }
      this.rating = Math.ceil(rating)
      this.state.item_rating = this.rating * 20
      webapi.library_track_set_rating(this.now_playing.track_id, this.rating * 20)
    },

    close_usermark_upd: function (args) {
      this.usermark = args.value
      this.show_details_modal = false
    },

    open_dialog: function (item) {
      this.selected_item = item
      this.show_details_modal = true
    }
  },

  watch: {
    'state' () {
      if (this.interval_id > 0) {
        window.clearTimeout(this.interval_id)
        this.interval_id = 0
      }
      this.item_progress_ms = this.state.item_progress_ms
      if (this.state.state === 'play') {
        this.interval_id = window.setInterval(this.tick, 1000)
      }
    },

    'now_playing' () {
      if (this.now_playing.track_id !== undefined) {
        webapi.library_track(this.now_playing.track_id).then((response) => {
          this.rating = response.data.rating / 20
          this.usermark = response.data.usermark
        }).catch((err) => {
          this.rating = 0
          this.usermark = 0
          if (err.response.status === 404) {
            webapi.queue_remove(this.now_playing.id)
          }
        })
      }
    }
  }
}
</script>

<style>
</style>
