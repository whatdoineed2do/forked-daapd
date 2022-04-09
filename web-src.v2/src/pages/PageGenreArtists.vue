<template>
  <div>
    <tabs-music></tabs-music>

    <index-list :index="artists_list.indexList"></index-list>

    <content-with-heading>
      <template slot="options">
        <index-button-list :index="artists_list.indexList"></index-button-list>

        <div class="columns">
          <div class="column">
            <p class="heading" style="margin-bottom: 24px;">Sort by</p>
            <dropdown-menu v-model="sort" :options="sort_options"></dropdown-menu>
          </div>
        </div>
      </template>
      <template slot="heading-left">
        <p class="title is-4">{{ genre }}</p>
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
        <p class="heading has-text-centered-mobile">{{ artists.total }} artists | <a class="has-text-link" @click="open_albums">albums</a> | <a class="has-text-link" @click="open_tracks">tracks</a> | <a class="has-text-link" @click="open_composers">composers</a></p>
        <list-artists :artists="artists_list"></list-artists>
        <modal-dialog-genre :show="show_genre_details_modal" :genre="modal_obj" @close="show_genre_details_modal = false" />
      </template>
    </content-with-heading>
  </div>
</template>

<script>
import { LoadDataBeforeEnterMixin } from './mixin'
import ContentWithHeading from '@/templates/ContentWithHeading'
import TabsMusic from '@/components/TabsMusic'
import IndexButtonList from '@/components/IndexButtonList'
import ListArtists from '@/components/ListArtists'
import DropdownMenu from '@/components/DropdownMenu'
import ModalDialogGenre from '@/components/ModalDialogGenre'
import IndexList from '@/components/IndexList'
import webapi from '@/webapi'
import * as types from '@/store/mutation_types'
import Artists from '@/lib/Artists'

const artistsData = {
  load: function (to) {
    return webapi.library_genre_artists(to.params.genre)
  },

  set: function (vm, response) {
    vm.genre = vm.$route.params.genre
    vm.artists = response.data.artists
  }
}

export default {
  name: 'PageGenreArtists',
  mixins: [LoadDataBeforeEnterMixin(artistsData)],
  components: { ContentWithHeading, TabsMusic, IndexButtonList, ListArtists, IndexList, ModalDialogGenre, DropdownMenu },

  data () {
    return {
      artists: { items: [] },
      genre: '',
      sort_options: ['Name', 'Recently added'],

      show_details_modal: false,

      show_genre_details_modal: false
    }
  },

  computed: {
    artists_list () {
      return new Artists(this.artists.items, {
        hideSingles: this.hide_singles,
        hideSpotify: this.hide_spotify,
        sort: this.sort,
        group: true
      })
    },

    sort: {
      get () {
        return this.$store.state.artists_sort
      },
      set (value) {
        this.$store.commit(types.ARTISTS_SORT, value)
      }
    },

    modal_obj () {
      return {
        name: this.genre,
        album_count: this.artists.items.reduce((acc, item) => {
          acc += item.album_count
          return acc
        }, 0),
        artist_count: this.artists.items.length,
        track_count: this.artists.items.reduce((acc, item) => {
          acc += item.track_count
          return acc
        }, 0),
        uri: this.artists.items.map(a => a.uri).join(',')
      }
    }
  },

  methods: {
    open_albums: function () {
      this.show_details_modal = false
      this.$router.push({ name: 'Genre', params: { genre: this.genre } })
    },

    open_composers: function () {
      this.show_details_modal = false
      this.$router.push({ name: 'Composers', params: { genre: this.genre } })
    },

    open_tracks: function () {
      this.show_details_modal = false
      this.$router.push({ name: 'GenreTracks', params: { genre: this.genre } })
    },

    open_artist: function (artist) {
      this.$router.push({ path: '/music/artists/' + artist.id })
    },

    play: function () {
      webapi.player_play_expression('genre is "' + this.genre + '" and media_kind is music', true)
    },

    play_artist: function (position) {
      webapi.player_play_expression('genre is "' + this.genre + '" and media_kind is music', false, position)
    },

    scrollToTop: function () {
      window.scrollTo({ top: 0, behavior: 'smooth' })
    }
  }
}
</script>

<style>
</style>
