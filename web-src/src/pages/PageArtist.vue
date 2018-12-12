<template>
  <div>
    <tabs-music></tabs-music>

    <index-list :index="index_list"></index-list>

  <content-with-heading>
      <template slot="options">
        <div class="columns">
          <div class="column">
            <p class="heading" style="margin-bottom: 24px;">Sort by</p>
            <dropdown-menu v-model="sort" :options="sort_options"></dropdown-menu>
          </div>
        </div>
      </template>
    <template slot="heading-left">
      <p class="title is-4">{{ artist.name }}</p>
    </template>
    <template slot="heading-right">
      <div class="buttons is-centered">
        <a class="button is-small is-light is-rounded" @click="show_artist_details_modal = true">
          <span class="icon"><i class="mdi mdi-dots-horizontal mdi-18px"></i></span>
        </a>
        <a class="button is-small is-dark is-rounded" @click="play">
          <span class="icon"><i class="mdi mdi-shuffle"></i></span> <span>Shuffle</span>
        </a>
      </div>
    </template>
    <template slot="content">
      <p class="heading has-text-centered-mobile">{{ artist.album_count }} albums | <a class="has-text-link" @click="open_tracks">{{ track_count }} tracks</a></p>
      <list-albums :albums="albums_list"></list-albums>
      <modal-dialog-artist :show="show_artist_details_modal" :artist="artist" @close="show_artist_details_modal = false" />
    </template>
  </content-with-heading>
  </div>
</template>

<script>
import { LoadDataBeforeEnterMixin } from './mixin'
import ContentWithHeading from '@/templates/ContentWithHeading'
import ListAlbums from '@/components/ListAlbums'
import TabsMusic from '@/components/TabsMusic'
import ModalDialogArtist from '@/components/ModalDialogArtist'
import DropdownMenu from '@/components/DropdownMenu'
import IndexList from '@/components/IndexList'
import webapi from '@/webapi'
import * as types from '@/store/mutation_types'
import Albums from '@/lib/Albums'

const artistData = {
  load: function (to) {
    return Promise.all([
      webapi.library_artist(to.params.artist_id),
      webapi.library_artist_albums(to.params.artist_id)
    ])
  },

  set: function (vm, response) {
    vm.name = response[0].data.name
    vm.id = response[0].data.id
    vm.artist = response[0].data.items
    vm.albums = response[1].data

    vm.consolidated_artist = {
      id: vm.id,
      name: vm.name,
      album_count: vm.albums.items.length,
      track_count: vm.track_count,
      uri: vm.albums.items.map(a => a.uri).join(',')
    }
  }
}

export default {
  name: 'PageArtist',
  mixins: [LoadDataBeforeEnterMixin(artistData)],
  components: { ContentWithHeading, ListAlbums, ModalDialogArtist, DropdownMenu, TabsMusic, IndexList },

  data () {
    return {
      name: '',
      id: '',
      consolidated_artist: {},
      artist: [], // can be multiple entries if compilation album
      albums: { items: [] },

      sort_options: ['Name', 'Release date'],
      show_artist_details_modal: false
    }
  },

  computed: {
    index_list () {
      return [...new Set(this.albums.items
        .map(album => album.name_sort.charAt(0).toUpperCase()))]
    },

    track_count () {
      return this.albums.items.reduce((acc, item) => {
        acc += item.track_count
        return acc
      }, 0)
    },

    albums_list () {
      return new Albums(this.albums.items, {
        sort: this.sort,
        group: false
      })
    },

    sort: {
      get () {
        return this.$store.state.artist_albums_sort
      },
      set (value) {
        this.$store.commit(types.ARTIST_ALBUMS_SORT, value)
      }
    }
  },

  methods: {
    open_tracks: function () {
      this.$router.push({ path: '/music/artists/' + this.id + '/tracks' })
    },

    play: function () {
      webapi.player_play_uri(this.albums.items.map(a => a.uri).join(','), true)
    }
  }
}
</script>

<style>
</style>
