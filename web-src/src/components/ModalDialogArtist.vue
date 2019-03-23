<template>
  <div>
    <transition name="fade">
      <div class="modal is-active" v-if="show">
        <div class="modal-background" @click="$emit('close')"></div>
        <div class="modal-content fd-modal-card">
          <div class="card">
            <div class="card-content">
              <p class="title is-4">
                <a class="has-text-link" @click="open_artist">{{ artist.name }}</a>
              </p>
              <div class="content is-small">
                <p>
                  <span class="heading">Albums</span>
                  <a class="title is-6 has-text-link" @click="open_artist">{{ artist.album_count }}</a>
                </p>
                <p>
                  <span class="heading">Tracks</span>
                  <a class="title is-6 has-text-link" @click="open_tracks">{{ artist.track_count }}</a>
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
import webapi from '@/webapi'

export default {
  name: 'ModalDialogArtist',
  props: [ 'show', 'artist' ],

  methods: {
    play: function () {
      this.$emit('close')
      webapi.player_play_uri(this.artist.uri, false)
    },

    queue_add: function () {
      this.$emit('close')
      webapi.queue_add(this.artist.uri)
    },

    queue_add_next: function () {
      this.$emit('close')
      webapi.queue_add_next(this.artist.uri)
    },

    open_artist: function () {
      this.$emit('close')
      this.$router.push({ path: '/music/artists/' + this.artist.id })
    },

    open_tracks: function () {
      this.$emit('close')
      this.$router.push({ path: '/music/artists/' + this.artist.id + '/tracks' })
    }
  }
}
</script>

<style>
</style>
