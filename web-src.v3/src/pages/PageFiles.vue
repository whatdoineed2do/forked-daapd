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
                    <span class="icon is-small"><mdicon name="folder" size="16"/></span>
                    <span>Directories ({{ files.directories.length }})</span>
                  </a>
                </li>
                <li :class="[ view === 'file_view' ? 'is-active' : '']">
                  <a @click="view='file_view'">
                    <span class="icon is-small"><mdicon name="file" size="16"/></span>
                    <span>Files ({{ files.tracks.items.length }})</span>
                  </a>
                </li>
                <li :class="[ view === 'pls_view' ? 'is-active' : '']">
                  <a @click="view='pls_view'">
                    <span class="icon is-small"><mdicon name="library-music" size="16"/></span>
                    <span>Playlists ({{ files.playlists.items.length }})</span>
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
      <template #heading-left>
        <p class="title is-4">Files</p>
        <p class="title is-7 has-text-grey">
          {{ current_directory }}
        </p>
      </template>
      <template #heading-right>
        <div class="buttons is-centered">
          <a
            class="button is-small is-light is-rounded"
            @click="open_directory_dialog({ path: current_directory })"
          >
            <span class="icon"
              ><mdicon name="dots-horizontal" size="16"
            /></span>
          </a>
          <a class="button is-small is-dark is-rounded" @click="play">
            <span class="icon"><mdicon name="play" size="16" /></span>
            <span>Play</span>
          </a>
        </div>
      </template>
      <template #content>
        <list-directories :directories="dirs_list" v-if="view === 'dir_view'"/>

	<list-playlists :playlists="playlists_list" v-if="view === 'pls_view'"/>

        <list-tracks
          :tracks="tracks_list"
          :expression="play_expression"
          :show_icon="true"
          @usermark-updated="usermark_upd"
	  v-if="view === 'file_view'"
        />

        <modal-dialog-directory
          :show="show_directory_details_modal"
          :directory="selected_directory"
          @close="show_directory_details_modal = false"
        />
      </template>
    </content-with-heading>
  </div>
</template>

<script>
import ContentWithHeading from '@/templates/ContentWithHeading.vue'
import ModalDialogDirectory from '@/components/ModalDialogDirectory.vue'
import ListDirectories from '@/components/ListDirectories.vue'
import ListPlaylists from '@/components/ListPlaylists.vue'
import ListTracks from '@/components/ListTracksWHeadings.vue'
import webapi from '@/webapi'
import IndexList from '@/components/IndexList.vue'
import { GroupByList, byName, byPath, noop } from '@/lib/GroupByList'

const dataObject = {
  load: function (to) {
    if (to.query.directory) {
      return webapi.library_files(to.query.directory)
    }
    return Promise.resolve()
  },

  set: function (vm, response) {
    vm.view = 'dir_view'
    if (response) {
      vm.files = response.data

      vm.tracks_list = new GroupByList(vm.files.tracks)
      vm.tracks_list.group(byPath('path'))

      vm.playlists_list = new GroupByList(vm.files.playlists)
      vm.playlists_list.group(byPath('path'))

      if (vm.files.tracks.items.length > 0) {
        vm.view = 'file_view'
      } else if (vm.files.directories.length > 0) {
        vm.view = 'dir_view'
      } else if (vm.files.playlists.items.length > 0) {
        vm.view = 'pls_view'
      }
    } else {
      vm.files = {
        directories: vm.$store.state.config.directories.map((dir) => {
          return { path: dir }
        }),
        tracks: { items: [] },
        playlists: { items: [] }
      }
    }
    vm.dirs_list = new GroupByList({ items: vm.files.directories})
    vm.dirs_list.group(byPath('path'))
  }
}

export default {
  name: 'PageFiles',
  components: {
    ModalDialogDirectory,
    IndexList,
    ContentWithHeading,
    ListDirectories,
    ListPlaylists,
    ListTracks
  },

  beforeRouteEnter(to, from, next) {
    dataObject.load(to).then((response) => {
      next((vm) => dataObject.set(vm, response))
    })
  },
  beforeRouteUpdate(to, from, next) {
    const vm = this
    dataObject.load(to).then((response) => {
      dataObject.set(vm, response)
      next()
    })
  },

  data() {
    return {
      files: {
        directories: [],
        tracks: { items: [] },
        playlists: { items: [] }
      },

      view: '',

      dirs_list: new GroupByList(),
      tracks_list: new GroupByList(),
      playlist_list: new GroupByList()
    }
  },

  computed: {
    current_directory() {
      if (this.$route.query && this.$route.query.directory) {
        return this.$route.query.directory
      }
      return '/'
    },

    index_list () {
      let items = []

      if (this.view === 'pls_view') {
        return this.playlists_list.indexList
      }

      if (this.view === 'file_view') {
        return this.tracks_list.indexList
      }

      if (this.view === 'dir_view') {
        return this.dirs_list.indexList
      }

      return [...new Set()]
    },

    play_expression() {
      return (
        'path starts with "' + this.current_directory + '" order by path asc'
      )
    }
  },

  methods: {
    basename: function (path) {
      return path.slice(this.current_directory === '/' ? path.lastIndexOf('/') + 1 : this.current_directory.length + 1, path.length)
    },

    usermark_upd: function (args) {
      this.tracks_list.items.find(e => e.id === args.track_id).usermark = args.value
    },

    play: function () {
      webapi.player_play_expression(this.play_expression, false)
    }
  }
}
</script>

<style></style>
