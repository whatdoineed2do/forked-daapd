<template>
  <template
    v-for="(track, index) in tracks"
    :key="track.item.id"
    class="media"
    :class="{ 'with-progress': show_progress }"
  >
   <div v-if="!track.isItem && !hide_group_title" class="mt-6 mb-5 py-2">
      <span
        :id="'index_' + groupKey(track)"
        class="tag is-info is-light is-small has-text-weight-bold fd-has-action"
        @click.prevent.stop="open_group_dialog(index, track)"
        >{{ track.groupKey }}</span
      >
    </div>
    <div v-else-if="track.isItem" class="media" @click="play_track(index, track)">
      <figure v-if="show_icon" class="media-left fd-has-action">
        <span class="icon">
          <mdicon name="file-outline" size="16" />
        </span>
      </figure>
      <div class="media-content fd-has-action is-clipped">
        <div style="margin-top: 0.7rem">
	  <h1
	    class="title is-6"
	    :class="{
	      'has-text-grey':
		track.item.usermark > 0 ||
		track.item.media_kind === 'podcast' && track.item.play_count > 0,
	      'is-italic':
		track.item.usermark > 0
	    }"
	  >
            {{ track.item.title }}
	  </h1>
          <h2 class="subtitle is-7 has-text-grey">
            <b>{{ track.item.artist }}</b>
          </h2>
          <h2 class="subtitle is-7 has-text-grey">
            <b>{{ track.item.album }}</b>
          </h2>
	  <progress-bar
	    v-if="show_progress"
	    :max="track.length_ms"
	    :value="track.seek_ms"
	  />
        </div>
      </div>
      <div class="media-right" style="padding-top: 0.7rem">
        <a @click.prevent.stop="open_dialog(track.item)">
          <span class="icon has-text-dark"
            ><mdicon name="dots-vertical" size="16"
          /></span>
        </a>
      </div>
    </div>
  </template>

  <teleport to="#app">
    <modal-dialog-track
      :show="show_details_modal"
      :track="selected_track"
      @close="show_details_modal = false"
      @play-count-changed="$emit('play-count-changed')"
      @usermark-updated="usermark_upd"
    />
    <modal-dialog-tracks
      :show="show_group_details_modal"
      :title="selected_tracks.title"
      :tracks="selected_tracks"
      @close="show_group_details_modal = false"
      @play-count-changed="$emit('play-count-changed')"
    />
  </teleport>
</template>

<script>
import ModalDialogTrack from '@/components/ModalDialogTrack.vue'
import ModalDialogTracks from '@/components/ModalDialogTracks.vue'
import ProgressBar from '@/components/ProgressBar.vue'
import webapi from '@/webapi'

export default {
  name: 'ListTracksWHeadings',
  components: { ModalDialogTrack, ModalDialogTracks, ProgressBar },

  props: ['tracks', 'uris', 'expression', 'show_progress', 'show_icon', 'hide_group_title'],
  emits: ['play-count-changed', 'usermark-updated'],

  data() {
    return {
      show_group_details_modal: false,
      selected_tracks: { title: '', items: [] },

      show_details_modal: false,
      selected_track: {}
    }
  },

  methods: {
    play_track: function (position, track) {
      if (this.uris) {
        webapi.player_play_uri(this.uris, false, position)
      } else if (this.expression) {
        webapi.player_play_expression(this.expression, false, position)
      } else {
       webapi.player_play_uri(track.item.uri, false)
      }
    },

    usermark_upd: function (args) {
      this.$emit('usermark-updated', args)
    },

    groupKey: function (where) {
      if (where.groupKey) return where.groupKey.replace(' ', '')

      return ''
    },

    open_group_dialog: function (index, track) {
      this.selected_tracks.title = track.groupKey
      this.selected_tracks.items = this.tracks.itemsByGroup[track.groupKey]
      this.show_group_details_modal = true
    },

    open_dialog: function (track) {
      this.selected_track = track
      this.show_details_modal = true
    }
  }
}
</script>

<style></style>
