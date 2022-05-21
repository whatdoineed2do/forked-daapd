<template>
  <div>
    <transition name="fade">
      <div class="modal is-active" v-if="show">
        <div class="modal-background" @click="$emit('close')"></div>
        <div class="modal-content fd-modal-card">
          <div class="card">
            <div class="card-content">
              <p class="title is-4"> {{ title }}</p>
              <div class="content is-small">
                <p>
                  <span class="heading">Albums</span>
                  <span class="title is-6">{{ album_count }}</span>
                </p>
                <p>
                  <span class="heading">Artists</span>
                  <span class="title is-6">{{ artist_count }}</span>
                </p>
                <p>
                  <span class="heading">Tracks</span>
                  <span class="title is-6">{{ tracks.items.length }}</span>
                </p>
              </div>
            </div>
            <footer class="card-footer">
              <a class="card-footer-item has-text-dark" @click="queue_add">
                <span class="icon"
                  ><mdicon name="playlist-plus" size="16"
                /></span>
                <span class="is-size-7">Add</span>
              </a>
              <a class="card-footer-item has-text-dark" @click="queue_add_next">
                <span class="icon"
                  ><mdicon name="playlist-play" size="16"
                /></span>
                <span class="is-size-7">Add Next</span>
              </a>
              <a class="card-footer-item has-text-dark" @click="play_track">
                <span class="icon"><mdicon name="play" size="16" /></span>
                <span class="is-size-7">Play</span>
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
  name: 'ModalDialogTracks',
  props: ['show', 'title', 'tracks'],
  emits: ['close', 'play-count-changed'],

  computed: {
    uris () {
      return this.tracks.items.map(a => a.uri).join(',')
    },
    artist_count () {
      return new Set(this.tracks.items.map(a => a.artist_id)).size
    },
    album_count () {
      return new Set(this.tracks.items.map(a => a.album_id)).size
    }
  },

  methods: {
    play: function () {
      this.$emit('close')
      webapi.player_play_uri(this.uris, false)
    },

    queue_add: function () {
      this.$emit('close')
      webapi.queue_add(this.uris)
    },

    queue_add_next: function () {
      this.$emit('close')
      webapi.queue_add_next(this.uris)
    }
  }
}
</script>

<style>
</style>
