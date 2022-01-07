<template>
  <div>
    <tabs-music></tabs-music>

    <index-list :index="index_list"></index-list>

    <content-with-heading>
      <template slot="options">
        <index-button-list :index="index_list"></index-button-list>
      </template>
      <template slot="heading-left">
        <p class="title is-4">{{ name }}</p>
      </template>
      <template slot="heading-right">
        <div class="buttons is-centered">
          <star-rating v-model="min_rating"
            :star-size="17"
            :show-rating="false"
            :max-rating="5"
            :increment="0.5"
            @rating-selected="show_rating"></star-rating>
          <a class="button is-small is-light is-rounded" @click="show_artist_details_modal = true">
            <span class="icon"><i class="mdi mdi-dots-horizontal mdi-18px"></i></span>
          </a>
          <a class="button is-small is-dark is-rounded" @click="play">
            <span class="icon"><i class="mdi mdi-shuffle"></i></span> <span>Shuffle</span>
          </a>
        </div>
      </template>
      <template slot="content">
        <p class="heading has-text-centered-mobile"><a class="has-text-link" @click="open_artist">{{ album_count }} albums</a> | {{ track_count }} tracks</p>
        <list-tracks :tracks="rated_tracks" @rating-updated="rating_upd" @usermark-updated="usermark_upd"></list-tracks>
        <modal-dialog-track :show="show_details_modal" :track="selected_track" @close="show_details_modal = false" />
        <modal-dialog-artist :show="show_artist_details_modal" :artist="modal_artist_obj" @close="show_artist_details_modal = false" />
      </template>
    </content-with-heading>
  </div>
</template>

<script>
import { LoadDataBeforeEnterMixin } from './mixin'
import ContentWithHeading from '@/templates/ContentWithHeading'
import IndexButtonList from '@/components/IndexButtonList'
import ListTracks from '@/components/ListTracks'
import ModalDialogTrack from '@/components/ModalDialogTrack'
import ModalDialogArtist from '@/components/ModalDialogArtist'
import TabsMusic from '@/components/TabsMusic'
import IndexList from '@/components/IndexList'
import StarRating from 'vue-star-rating'
import webapi from '@/webapi'

const tracksData = {
  load: function (to) {
    return Promise.all([
      webapi.library_artist(to.params.artist_id),
      webapi.library_artist_tracks(to.params.artist_id)
    ])
  },

  set: function (vm, response) {
    vm.name = response[0].data.name
    vm.id = response[0].data.id
    vm.artist = response[0].data
    vm.tracks = response[1].data.tracks
  }
}

export default {
  name: 'PageArtistTracks',
  mixins: [LoadDataBeforeEnterMixin(tracksData)],
  components: { ContentWithHeading, ListTracks, IndexButtonList, ModalDialogArtist, ModalDialogTrack, IndexList, TabsMusic, StarRating },

  data () {
    return {
      name: '',
      id: '',
      artist: {},
      tracks: { items: [] },
      min_rating: 0,

      show_details_modal: false,
      selected_track: {},

      show_artist_details_modal: false
    }
  },

  computed: {
    track_count () {
      return this.tracks.items.length
    },

    album_count () {
      return new Set(this.tracks.items.map(track => track.album_id)).size
    },

    rated_tracks () {
      return this.tracks.items.filter(track => track.rating >= this.min_rating)
    },

    modal_artist_obj () {
      var tracks = this.min_rating === 0 ? this.tracks.items : this.tracks.items.filter(a => a.rating >= this.min_rating)
      return {
        id: this.id,
        name: this.name,
        album_count: new Set(tracks.map(track => track.album_id)).size,
        track_count: tracks.length,
        uri: tracks.map(a => a.uri).join(',')
      }
    },

    index_list () {
      return [...new Set(this.tracks.items
        .map(track => track.title_sort.charAt(0).toUpperCase()))]
    },

    track_uris () {
      return this.tracks.items.map(a => a.uri).join(',')
    }
  },

  methods: {
    show_rating: function (rating) {
      if (rating === 0.5) {
        rating = 0
      }
      this.min_rating = Math.ceil(rating) * 20
    },

    open_artist: function () {
      this.show_details_modal = false
      this.$router.push({ path: '/music/artists/' + this.id })
    },

    rating_upd: function (args) {
      const idx = this.tracks.items.findIndex(track => track.id === args.track_id)
      if (idx > -1) {
        this.tracks.items[idx].rating = args.rating
      }
    },

    usermark_upd: function (args) {
      this.tracks.items.find(e => e.id === args.track_id).usermark = args.value
    },

    play: function () {
      webapi.player_play_uri(this.tracks.items.map(a => a.uri).join(','), true)
    }
  }
}
</script>

<style>
</style>
