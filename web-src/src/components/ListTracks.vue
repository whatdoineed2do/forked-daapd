<template>
  <div>
    <div v-if="is_grouped">
      <div v-for="idx in tracks.indexList" :key="idx" class="mb-6">
        <span class="tag is-info is-light is-small has-text-weight-bold" :id="'index_' + idx">{{ idx }}</span>
        <list-item-track v-for="track in tracks.grouped[idx]"
            :key="track.id"
            :track="track"
            @click="open_dialog(track)">
            <template slot="actions">
                <a @click="open_dialog(track)">
                <span class="icon has-text-dark"><i class="mdi mdi-dots-vertical mdi-18px"></i></span>
                </a>
            </template>
        </list-item-track>
      </div>
    </div>
    <div v-else>
    <list-item-track v-for="(track, index) in tracks" :key="track.id" :track="track" @click="play_track(index, track)">
      <template slot="actions">
        <a @click="open_dialog(track)">
          <span class="icon has-text-dark"><i class="mdi mdi-dots-vertical mdi-18px"></i></span>
        </a>
      </template>
    </list-item-track>
    </div>
    <modal-dialog-track :show="show_details_modal" :track="selected_track" @close="show_details_modal = false" @rating-updated="rating_upd" @usermark-updated="usermark_upd" />
  </div>
</template>

<script>
import ListItemTrack from '@/components/ListItemTrack'
import ModalDialogTrack from '@/components/ModalDialogTrack'
import webapi from '@/webapi'
import Tracks from '@/lib/Tracks'

export default {
  name: 'ListTracks',
  components: { ListItemTrack, ModalDialogTrack },

  props: ['tracks', 'uris', 'expression'],

  data () {
    return {
      show_details_modal: false,
      selected_track: {}
    }
  },

  computed: {
    media_kind_resolved: function () {
      return this.media_kind ? this.media_kind : this.selected_track.media_kind
    },

    tracks_list: function () {
      if (Array.isArray(this.tracks)) {
        return this.tracks
      }
      return this.tracks.sortedAndFiltered
    },

    is_grouped: function () {
      return (this.tracks instanceof Tracks && this.tracks.options.group)
    }
  },

  methods: {
    play_track: function (position, track) {
      if (this.uris) {
        webapi.player_play_uri(this.uris, false, position)
      } else if (this.expression) {
        webapi.player_play_expression(this.expression, false, position)
      } else {
        webapi.player_play_uri(track.uri, false)
      }
    },

    rating_upd: function (args) {
      this.$emit('rating-updated', args)
    },

    usermark_upd: function (args) {
      this.$emit('usermark-updated', args)
    },

    open_dialog: function (track) {
      this.selected_track = track
      this.show_details_modal = true
    }
  }
}
</script>

<style>
</style>
