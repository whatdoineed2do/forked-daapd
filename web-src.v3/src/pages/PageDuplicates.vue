<template>
  <div class="fd-page-with-tabs">
    <tabs-music />

    <index-list :index="tracks.indexList"></index-list>

  <div>
    <content-with-heading>
      <template #options>
        <index-button-list :index="index_list" />
	<div class="columns">
	  <div class="column">
	    <p class="heading" style="margin-bottom: 24px">Sort by</p>
	    <dropdown-menu
	      v-model="selected_groupby_option_name"
	      :options="groupby_option_names"
	    />
	  </div>
	</div>
      </template>
      <template #heading-left>
        <p class="title is-4">
          {{ dup_type }}
        </p>
      </template>
      <template #heading-right>
        <div class="buttons is-centered">
          <a
            class="button is-small is-light is-rounded"
            @click="show_details_modal = true"
          >
            <span class="icon"
              ><mdicon name="dots-horizontal" size="16"
            /></span>
          </a>
          <a class="button is-small is-dark is-rounded" @click="play">
            <span class="icon"><mdicon name="shuffle" size="16" /></span>
            <span>Shuffle</span>
          </a>
        </div>
      </template>
      <template #content>
        <p class="heading has-text-centered-mobile">
          <a class="has-text-link" @click="select_tracks_meta"
            >{{ tracks_list_meta.total }} Meta</a
          >
          | <a class="has-text-link" @click="select_tracks_hash"> {{ tracks_list_hash.total }} Hash</a>
        </p>
        <list-tracks :tracks="tracks" :uris="track_uris" @usermark-updated="usermark_upd" />
	<modal-dialog-tracks
         :show="show_details_modal"
	 :uris="track_uris"
	 :title="dup_type"
	 :album_count="album_count"
	 :artist_count="artist_count"
	 :track_count="track_count"
         @close="show_details_modal = false"
        />
      </template>
    </content-with-heading>
  </div>
  </div>
</template>

<script>
import ContentWithHeading from '@/templates/ContentWithHeading.vue'
import TabsMusic from '@/components/TabsMusic.vue'
import DropdownMenu from '@/components/DropdownMenu.vue'
import IndexList from '@/components/IndexList.vue'
import IndexButtonList from '@/components/IndexButtonList.vue'
import ListTracks from '@/components/ListTracksWHeadings.vue'
import ModalDialogTracks from '@/components/ModalDialogTracks.vue'
import webapi from '@/webapi'
import * as types from '@/store/mutation_types'
import { bySortName, byName, GroupByList } from '@/lib/GroupByList'

const dataObject = {
  load: function (to) {
    return Promise.all([
      webapi.library_duplicates(),
      webapi.library_duplicates_by_meta(),
    ])
  },

  set: function (vm, response) {
    vm.tracks_list_hash = new GroupByList(response[0].data)
    vm.tracks_list_meta = new GroupByList(response[1].data)

    vm.tracks_list = vm.tracks_list_hash
    vm.dup_type = "Duplicates (hash)"
  }
}

export default {
  name: 'PageDuplicates',
  components: {
    ContentWithHeading,
    TabsMusic,
    DropdownMenu,
    IndexList,
    ListTracks,
    ModalDialogTracks,
    IndexButtonList
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
      tracks_list: new GroupByList(),
      tracks_list_hash: new GroupByList(),
      tracks_list_meta: new GroupByList(),

      dup_type: '',

      groupby_options: [
	{ name: 'Name', options: bySortName('title_sort') },
        { name: 'Album', options: bySortName('album_sort') },
        { name: 'Artist', options: bySortName('artist_sort') },
        { name: 'Hash', options: byName('audio_hash') }
      ],

      show_details_modal: false
    }
  },

  computed: {
    tracks() {
      const groupBy = this.groupby_options.find(
        (o) => o.name === this.selected_groupby_option_name
      )
      this.tracks_list.group(groupBy.options)

      return this.tracks_list
    },

    groupby_option_names() {
      return [...this.groupby_options].map((o) => o.name)
    },

    selected_groupby_option_name: {
      get() {
        return this.$store.state.tracks_sort
      },
      set(value) {
        this.$store.commit(types.TRACKS_SORT, value)
      }
    },

    dup_type() {
      return this.dup_type
    },

    track_uris() {
      return this.tracks.items.map((a) => a.uri).join(',')
    },

    album_count() {
      return new Set(this.tracks.items.map(e => e.album_id)).size
    },
    artist_count() {
      return new Set(this.tracks.items.map(e => e.artist_id)).size
    },
    track_count() {
      return this.tracks.items.length
    },

    index_list() {
      return this.tracks_list.indexList
    }
  },

  methods: {
    usermark_upd: function (args) {
      this.tracks.items.find(e => e.id === args.track_id).usermark = args.value
    },

    select_tracks_hash: function () {
      this.tracks_list = this.tracks_list_hash
      this.dup_type = "Duplicates (hash)"
    },

    select_tracks_meta: function () {
      this.tracks_list = this.tracks_list_meta
      this.dup_type = "Duplicates (meta)"
    },

    play: function () {
      webapi.player_play_uri(
        track_uris,
        true
      )
    }
  }
}
</script>

<style></style>
