<template>
  <div>
    <section class="section fd-tabs-section">
      <div class="container">
        <div class="columns is-centered">
          <div class="column is-four-fifths">
            <div class="tabs is-centered is-small">
              <ul>
                <li :class="[ view === 'dir_view' ? 'is-active' : '']">
                  <a @click="view='dir_view'">
                    <span class="icon is-small"><i class="mdi mdi-folder"></i></span>
                    <span>Directories ({{ count(files.directories) }})</span>
                  </a>
                </li>
                <li :class="[ view === 'file_view' ? 'is-active' : '']">
                  <a @click="view='file_view'">
                    <span class="icon is-small"><i class="mdi mdi-file"></i></span>
                    <span>Files ({{ count(files.tracks.items) }})</span>
                  </a>
                </li>
                <li :class="[ view === 'pls_view' ? 'is-active' : '']">
                  <a @click="view='pls_view'">
                    <span class="icon is-small"><i class="mdi mdi-library-music"></i></span>
                    <span>Playlists ({{ count(files.playlists.items) }})</span>
                  </a>
                </li>
              </ul>
            </div>
          </div>
        </div>
      </div>
    </section>

    <index-list :index="index_list"></index-list>

    <content-with-heading>
      <template slot="heading-left">
        <p class="title is-4">Files</p>
        <p class="title is-7 has-text-grey">{{ current_directory }}</p>
      </template>
      <template slot="heading-right">
        <div class="buttons is-centered">
          <a class="button is-small is-light is-rounded" @click="open_directory_dialog({ 'path': current_directory })">
            <span class="icon"><i class="mdi mdi-dots-horizontal mdi-18px"></i></span>
          </a>
          <a class="button is-small is-dark is-rounded" @click="play">
            <span class="icon"><i class="mdi mdi-play"></i></span> <span>Play</span>
          </a>
        </div>
      </template>
      <template slot="content">
        <div class="media" v-if="$route.query.directory" @click="open_parent_directory()">
          <figure class="media-left fd-has-action">
            <span class="icon">
              <i class="mdi mdi-subdirectory-arrow-left"></i>
            </span>
          </figure>
          <div class="media-content fd-has-action is-clipped">
            <h1 class="title is-6">..</h1>
          </div>
          <div class="media-right">
            <slot name="actions"></slot>
          </div>
        </div>

        <div v-if="view === 'dir_view'">
        <list-item-directory v-for="directory in files.directories" :key="directory.path" :directory="directory" :alt_index_id="basename(directory.path)" @click="open_directory(directory)">
        <template slot="actions">
          <a @click="open_directory_dialog(directory)">
            <span class="icon has-text-dark"><i class="mdi mdi-dots-vertical mdi-18px"></i></span>
          </a>
        </template>
        </list-item-directory>
        </div>

        <div v-if="view === 'pls_view'">
        <list-item-playlist v-for="playlist in files.playlists.items" :key="playlist.id" :playlist="playlist" @click="open_playlist(playlist)">
          <template slot="icon">
            <span class="icon">
              <i class="mdi mdi-library-music"></i>
            </span>
          </template>
          <template slot="actions">
            <a @click="open_playlist_dialog(playlist)">
              <span class="icon has-text-dark"><i class="mdi mdi-dots-vertical mdi-18px"></i></span>
            </a>
          </template>
        </list-item-playlist>
        </div>

        <div v-if="view === 'file_view'">
        <list-item-track v-for="(track, index) in files.tracks.items" :key="track.id" :track="track" :alt_index_id="basename(track.path)" @click="play_track(index)">
          <template slot="icon">
            <span class="icon">
              <i class="mdi mdi-file-outline"></i>
            </span>
          </template>
          <template slot="actions">
            <a @click="open_track_dialog(track)">
              <span class="icon has-text-dark"><i class="mdi mdi-dots-vertical mdi-18px"></i></span>
            </a>
          </template>
        </list-item-track>
        </div>

        <modal-dialog-directory :show="show_directory_details_modal" :directory="selected_directory" @close="show_directory_details_modal = false" />
        <modal-dialog-playlist :show="show_playlist_details_modal" :playlist="selected_playlist" @close="show_playlist_details_modal = false" />
        <modal-dialog-track :show="show_track_details_modal" :track="selected_track" @close="show_track_details_modal = false" />
      </template>
    </content-with-heading>
  </div>
</template>

<script>
import { LoadDataBeforeEnterMixin } from './mixin'
import ContentWithHeading from '@/templates/ContentWithHeading'
import ListItemDirectory from '@/components/ListItemDirectory'
import ListItemPlaylist from '@/components/ListItemPlaylist'
import ListItemTrack from '@/components/ListItemTrack'
import ModalDialogDirectory from '@/components/ModalDialogDirectory'
import ModalDialogPlaylist from '@/components/ModalDialogPlaylist'
import ModalDialogTrack from '@/components/ModalDialogTrack'
import IndexList from '@/components/IndexList'
import webapi from '@/webapi'

const filesData = {
  load: function (to) {
    if (to.query.directory) {
      return webapi.library_files(to.query.directory)
    }
    return Promise.resolve()
  },

  set: function (vm, response) {
    vm.view = 'file_view'
    if (response) {
      vm.files = response.data
      if (vm.count(vm.files.tracks.items) > 0) {
        vm.view = 'file_view'
      } else if (vm.count(vm.files.directories) > 0) {
        vm.view = 'dir_view'
      } else if (vm.count(vm.files.playlists.items) > 0) {
        vm.view = 'pls_view'
      }
    } else {
      vm.view = 'dir_view'
      vm.files = {
        directories: vm.$store.state.config.directories.map(dir => { return { path: dir } }),
        tracks: { items: [] },
        playlists: { items: [] }
      }
    }
  }
}

export default {
  name: 'PageFiles',
  mixins: [LoadDataBeforeEnterMixin(filesData)],
  components: { ContentWithHeading, ListItemDirectory, ListItemPlaylist, ListItemTrack, ModalDialogDirectory, ModalDialogPlaylist, ModalDialogTrack, IndexList },

  data () {
    return {
      files: { directories: [], tracks: { items: [] }, playlists: { items: [] } },

      view: '',

      show_directory_details_modal: false,
      selected_directory: {},

      show_playlist_details_modal: false,
      selected_playlist: {},

      show_track_details_modal: false,
      selected_track: {}
    }
  },

  computed: {
    current_directory () {
      if (this.$route.query && this.$route.query.directory) {
        return this.$route.query.directory
      }
      return '/'
    },

    index_list () {
      var items = []

      if (this.view === 'file_view') {
        items = this.files.tracks.items
      } else if (this.view === 'dir_view') {
        items = this.files.directories
      }

      if (items.length === 0) {
        return [...new Set()]
      }
      return [...new Set(items
        .map(dirent => this.basename(dirent.path).charAt(0).toUpperCase()))]
    }
  },

  methods: {
    open_parent_directory: function () {
      const parent = this.current_directory.slice(0, this.current_directory.lastIndexOf('/'))
      if (parent === '' || this.$store.state.config.directories.includes(this.current_directory)) {
        this.$router.push({ path: '/files' })
      } else {
        this.$router.push({ path: '/files', query: { directory: this.current_directory.slice(0, this.current_directory.lastIndexOf('/')) } })
      }
    },

    open_directory: function (directory) {
      this.$router.push({ path: '/files', query: { directory: directory.path } })
    },

    open_directory_dialog: function (directory) {
      this.selected_directory = directory
      this.show_directory_details_modal = true
    },

    count: function (what) {
      if (what === undefined || what === null) {
        return 0
      }
      return what.length
    },

    basename: function (path) {
      return path.slice(this.current_directory === '/' ? path.lastIndexOf('/') + 1 : this.current_directory.length + 1, path.length)
    },

    play: function () {
      webapi.player_play_expression('path starts with "' + this.current_directory + '" order by path asc', false)
    },

    play_track: function (position) {
      webapi.player_play_uri(this.files.tracks.items.map(a => a.uri).join(','), false, position)
    },

    open_track_dialog: function (track) {
      this.selected_track = track
      this.show_track_details_modal = true
    },

    open_playlist: function (playlist) {
      this.$router.push({ path: '/playlists/' + playlist.id + '/tracks' })
    },

    open_playlist_dialog: function (playlist) {
      this.selected_playlist = playlist
      this.show_playlist_details_modal = true
    }
  }
}
</script>

<style>
</style>
