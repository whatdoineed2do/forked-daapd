<template>
  <div class="fd-page-with-tabs">
    <tabs-music />

    <index-list :index="tracks.indexList"></index-list>

  <div>
    <content-with-heading>
      <template #options>
        <index-button-list :index="tracks.indexList" />
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
          {{ artist.name }}
        </p>
      </template>
      <template #heading-right>
        <div class="buttons is-centered">
          <a
            class="button is-small is-light is-rounded"
            @click="show_artist_details_modal = true"
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
          <a class="has-text-link" @click="open_artist"
            >{{ album_count }} albums</a
          >
          | {{ tracks.count }} tracks
        </p>
        <list-tracks :tracks="tracks" :uris="track_uris" @usermark-updated="usermark_upd" />
        <modal-dialog-artist
          :show="show_artist_details_modal"
          :artist="modal_artist"
          @close="show_artist_details_modal = false"
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
import ModalDialogArtist from '@/components/ModalDialogArtist.vue'
import webapi from '@/webapi'
import * as types from '@/store/mutation_types'
import { bySortName, byYear, byRating, GroupByList } from '@/lib/GroupByList'

const dataObject = {
  load: function (to) {
    return Promise.all([
      webapi.library_artist(to.params.artist_id),
      webapi.library_artist_tracks(to.params.artist_id)
    ])
  },

  set: function (vm, response) {
    vm.artist = response[0].data
    vm.tracks_list = new GroupByList(response[1].data.tracks)
  }
}

export default {
  name: 'PageArtistTracks',
  components: {
    ContentWithHeading,
    TabsMusic,
    DropdownMenu,
    IndexList,
    ListTracks,
    IndexButtonList,
    ModalDialogArtist
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
      artist: { items: [] },

      tracks_list: new GroupByList(),

      groupby_options: [
        { name: 'Name', options: bySortName('title_sort') },
        { name: 'Rating', options: byRating('rating') },
        {
          name: 'Release date',
          options: byYear('date_released', {
            direction: 'asc',
            defaultValue: '0000'
          })
        },
        {
          name: 'Date Added',
          options: byYear('time_added', {
            direction: 'desc',
            defaultValue: '0000'
          })
        }
      ],

      show_artist_details_modal: false
    }
  },

  computed: {
   modal_artist() {
      return {
        id: this.id,
        name: this.name,
        album_count: this.album_count,
        track_count: this.tracks.count,
        uri: this.track_uris
      }
    },

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
        this.$store.commit(types.ARTIST_TRACKS_SORT, value)
      }
    },

    album_count () {
      return this.artist.items.reduce((acc, item) => {
        acc += item.album_count
        return acc
      }, 0)
    },

    track_uris() {
      return this.tracks.items.map((a) => a.uri).join(',')
    }
  },

  methods: {
    open_artist: function () {
      this.show_details_modal = false
      this.$router.push({ path: '/music/artists/' + this.artist.id })
    },

    usermark_upd: function (args) {
      this.tracks.items.find(e => e.id === args.track_id).usermark = args.value
    },

    play: function () {
      webapi.player_play_uri(
        this.tracks.items.map((a) => a.uri).join(','),
        true
      )
    }
  }
}
</script>

<style></style>
