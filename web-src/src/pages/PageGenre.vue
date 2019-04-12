<template>
  <div>
    <tabs-music></tabs-music>

    <index-list :index="index_list"></index-list>

    <content-with-heading>
      <template slot="heading-left">
        <p class="title is-4">{{ name }}</p>
      </template>
      <template slot="heading-right">
        <div class="buttons is-centered">
          <a class="button is-small is-light is-rounded" @click="show_genre_details_modal = true">
            <span class="icon"><i class="mdi mdi-dots-horizontal mdi-18px"></i></span>
          </a>
          <a class="button is-small is-dark is-rounded" @click="play">
            <span class="icon"><i class="mdi mdi-shuffle"></i></span> <span>Shuffle</span>
          </a>
        </div>
      </template>
      <template slot="content">
        <p class="heading has-text-centered-mobile"><a class="has-text-link" @click="open_artists">artists</a> | {{ genre_albums.total }} albums | <a class="has-text-link" @click="open_tracks">tracks</a> | <a class="has-text-link" @click="open_composers">composers</a> </p>
        <p class="heading has-text-centered-mobile"><a class="has-text-link" @click="open_toptracks">top tracks</a></p>
        <list-item-albums v-for="album in genre_albums.items" :key="album.id" :album="album" @click="open_album(album)">
          <template slot="actions">
            <a @click="open_dialog(album)">
              <span class="icon has-text-dark"><i class="mdi mdi-dots-vertical mdi-18px"></i></span>
            </a>
          </template>
        </list-item-albums>
        <modal-dialog-album :show="show_details_modal" :album="selected_album" @close="show_details_modal = false" />
        <modal-dialog-genre :show="show_genre_details_modal" :genre="modal_obj" @close="show_genre_details_modal = false" />
      </template>
    </content-with-heading>
  </div>
</template>

<script>
import { LoadDataBeforeEnterMixin } from './mixin'
import ContentWithHeading from '@/templates/ContentWithHeading'
import TabsMusic from '@/components/TabsMusic'
import ListItemAlbums from '@/components/ListItemAlbum'
import ModalDialogAlbum from '@/components/ModalDialogAlbum'
import ModalDialogGenre from '@/components/ModalDialogGenre'
import IndexList from '@/components/IndexList'
import webapi from '@/webapi'

const genreData = {
  load: function (to) {
    return webapi.library_genre_albums(to.params.genre)
  },

  set: function (vm, response) {
    vm.name = vm.$route.params.genre
    vm.genre_albums = response.data.albums
    vm.tracks = vm.genre_albums.items.reduce((acc, item) => {
      acc += item.track_count
      return acc
    }, 0)
  }
}

export default {
  name: 'PageGenre',
  mixins: [ LoadDataBeforeEnterMixin(genreData) ],
  components: { ContentWithHeading, TabsMusic, ListItemAlbums, IndexList, ModalDialogAlbum, ModalDialogGenre },

  data () {
    return {
      name: '',
      genre_albums: { items: [] },
      tracks: 0,
      show_details_modal: false,
      selected_album: {},

      show_genre_details_modal: false
    }
  },

  computed: {
    modal_obj () {
      return {
        'name': this.name,
        'album_count': this.genre_albums.items.length,
        'artist_count': new Set(this.genre_albums.items.map(album => album.artist_id)).size,
        'track_count': this.tracks,
        'uri': this.genre_albums.items.map(a => a.uri).join(',')
      }
    },

    index_list () {
      return [...new Set(this.genre_albums.items
        .map(album => album.name_sort.charAt(0).toUpperCase()))]
    }
  },

  methods: {
    open_tracks: function () {
      this.show_details_modal = false
      this.$router.push({ name: 'GenreTracks', params: { genre: this.name } })
    },

    open_toptracks: function () {
      this.show_details_modal = false
      this.$router.push({ name: 'TopGenreTracks', params: { condition: 'genre is "' + this.name + '" and media_kind is music', id: this.name } })
    },

    open_artists: function () {
      this.show_details_modal = false
      this.$router.push({ name: 'GenreArtists', params: { genre: this.name } })
    },

    open_composers: function () {
      this.show_details_modal = false
      this.$router.push({ name: 'Composers', params: { genre: this.name } })
    },

    play: function () {
      webapi.player_play_expression('genre is "' + this.name + '" and media_kind is music', true)
    },

    open_album: function (album) {
      this.$router.push({ path: '/music/albums/' + album.id })
    },

    open_dialog: function (album) {
      this.selected_album = album
      this.show_details_modal = true
    }
  }
}
</script>

<style>
</style>
