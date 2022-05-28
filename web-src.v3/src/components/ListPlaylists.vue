<template>
  <template
    v-for="playlist in playlists"
    :key="playlist.itemId"
    class="media"
    :playlist="playlist"
  >
   <div v-if="!playlist.isItem && !hide_group_title" class="mt-6 mb-5 py-2">
      <span
        :id="'index_' + playlist.groupKey"
        class="tag is-info is-light is-small has-text-weight-bold"
        >{{ playlist.groupKey }}</span
      >
    </div>
    <div v-else-if="playlist.isItem" class="media" @click="open_playlist(playlist.item)">
      <figure vi-if="show_icon" class="media-left fd-has-action">
	<span class="icon">
	  <mdicon :name="icon_name(playlist.item)" size="16" />
	</span>
      </figure>
      <div class="media-content fd-has-action is-clipped">
	<div style="margin-top: 0.7rem">
	  <h1 class="title is-6">
	    {{ playlist.item.name }}
	  </h1>
	</div>
      </div>
      <div class="media-right" style="padding-top: 0.7rem">
	<a @click.prevent.stop="open_dialog(playlist.item)">
	  <span class="icon has-text-dark"
	    ><mdicon name="dots-vertical" size="16"
	  /></span>
	</a>
      </div>
    </div>
  </template>
  <teleport to="#app">
    <modal-dialog-playlist
      :show="show_details_modal"
      :playlist="selected_playlist"
      @close="show_details_modal = false"
    />
  </teleport>
</template>

<script>
import ModalDialogPlaylist from '@/components/ModalDialogPlaylist.vue'

export default {
  name: 'ListPlaylists',
  components: { ModalDialogPlaylist },

  props: ['playlists', 'show-icon', 'hide_group_title'],

  data() {
    return {
      show_details_modal: false,
      selected_playlist: {}
    }
  },

  methods: {
    open_playlist: function (playlist) {
      if (playlist.type !== 'folder') {
        this.$router.push({ path: '/playlists/' + playlist.id + '/tracks' })
      } else {
        this.$router.push({ path: '/playlists/' + playlist.id })
      }
    },

    open_dialog: function (playlist) {
      this.selected_playlist = playlist
      this.show_details_modal = true
    },

    icon_name: function (playlist) {
      if (playlist.type === 'folder') {
        return 'folder'
      } else if (playlist.type === 'rss') {
        return 'rss'
      } else {
        return 'music-box-multiple'
      }
    }
  }
}
</script>

<style></style>
