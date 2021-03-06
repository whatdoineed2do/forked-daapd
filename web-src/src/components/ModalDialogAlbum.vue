<template>
  <div>
    <transition name="fade">
      <div class="modal is-active" v-if="show">
        <div class="modal-background" @click="$emit('close')"></div>
        <div class="modal-content fd-modal-card">
          <div class="card">
            <div class="card-content">
              <cover-artwork
                :artwork_url="album.artwork_url"
                :artist="album.artist"
                :album="album.name"
                class="image is-square fd-has-margin-bottom fd-has-shadow" />
              <p class="title is-4">
                <a class="has-text-link" @click="open_album">{{ album.name }}</a>
              </p>
              <div class="buttons" v-if="media_kind_resolved === 'podcast'">
                <a class="button is-small" @click="mark_played">Mark as played</a>
                <a class="button is-small" v-if="show_remove_button" @click="$emit('remove-podcast')">Remove podcast</a>
              </div>
              <div class="content is-small">
                <p v-if="album.artist">
                  <span class="heading">Album artist</span>
                  <a class="title is-6 has-text-link" @click="open_artist">{{ album.artist }}</a>
                </p>
                <p v-if="album.date_released">
                  <span class="heading">Release date</span>
                  <span class="title is-6">{{ album.date_released | time('L') }}</span>
                </p>
                <p v-else-if="album.year > 0">
                  <span class="heading">Year</span>
                  <span class="title is-6">{{ album.year }}</span>
                </p>
                <p>
                  <span class="heading">Tracks</span>
                  <span class="title is-6">{{ album.track_count }}</span>
                </p>
                <p>
                  <span class="heading">Length</span>
                  <span class="title is-6">{{ album.length_ms | duration }}</span>
                </p>
                <p>
                  <span class="heading">Type</span>
                  <span class="title is-6">{{ album.media_kind }} - {{ album.data_kind }}</span>
                </p>
                <p>
                  <span class="heading">Added at</span>
                  <span class="title is-6">{{ album.time_added | time('L LT') }}</span>
                </p>
              </div>
            </div>
            <footer class="card-footer">
              <a class="card-footer-item has-text-dark" @click="queue_add">
                <span class="icon"><i class="mdi mdi-playlist-plus"></i></span> <span class="is-size-7">Add</span>
              </a>
              <a class="card-footer-item has-text-dark" @click="queue_add_next">
                <span class="icon"><i class="mdi mdi-playlist-play"></i></span> <span class="is-size-7">Add Next</span>
              </a>
              <a class="card-footer-item has-text-dark" @click="play">
                <span class="icon"><i class="mdi mdi-play"></i></span> <span class="is-size-7">Play</span>
              </a>
            </footer>
          </div>
        </div>
        <button class="modal-close is-large" aria-label="close" @click="$emit('close')"></button>
      </div>
    </transition>
  </div>
</template>

<script>
import CoverArtwork from '@/components/CoverArtwork'
import webapi from '@/webapi'

export default {
  name: 'ModalDialogAlbum',
  components: { CoverArtwork },
  props: ['show', 'album', 'media_kind', 'request_remove_button', 'new_tracks'],

  data () {
    return {
      artwork_visible: false
    }
  },

  computed: {
    artwork_url: function () {
      return webapi.artwork_url_append_size_params(this.album.artwork_url)
    },

    show_remove_button: function () {
      return this.request_remove_button === undefined || this.request_remove_button
    },

    media_kind_resolved: function () {
      return this.media_kind ? this.media_kind : this.album.media_kind
    }
  },

  methods: {
    play: function () {
      this.$emit('close')
      webapi.player_play_uri(this.album.uri, false)
    },

    queue_add: function () {
      this.$emit('close')
      webapi.queue_add(this.album.uri)
    },

    queue_add_next: function () {
      this.$emit('close')
      webapi.queue_add_next(this.album.uri)
    },

    open_album: function () {
      if (this.media_kind_resolved === 'podcast') {
        this.$router.push({ path: '/podcasts/' + this.album.id })
      } else if (this.media_kind_resolved === 'audiobook') {
        this.$router.push({ path: '/audiobooks/' + this.album.id })
      } else {
        this.$router.push({ path: '/music/albums/' + this.album.id })
      }
    },

    open_artist: function () {
      if (this.media_kind_resolved === 'podcast') {
        // No artist page for podcasts
      } else if (this.media_kind_resolved === 'audiobook') {
        this.$router.push({ path: '/audiobooks/artists/' + this.album.artist_id })
      } else {
        this.$router.push({ path: '/music/artists/' + this.album.artist_id })
      }
    },

    mark_played: function () {
      webapi.library_album_track_update(this.album.id, { play_count: 'played' }).then(({ data }) => {
        this.$emit('play-count-changed')
        this.$emit('close')
      })
    },

    artwork_loaded: function () {
      this.artwork_visible = true
    },

    artwork_error: function () {
      this.artwork_visible = false
    }
  }
}
</script>

<style>
</style>
