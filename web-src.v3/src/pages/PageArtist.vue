<template>
  <div class="fd-page-with-tabs">
    <tabs-music />

    <index-list :index="albums.indexList"></index-list>

  <content-with-heading>
    <template #options>
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
        {{ name }}
      </p>
    </template>
    <template #heading-right>
      <div class="buttons is-centered">
        <a
          class="button is-small is-light is-rounded"
          @click="show_artist_details_modal = true"
        >
          <span class="icon"><mdicon name="dots-horizontal" size="16" /></span>
        </a>
        <a class="button is-small is-dark is-rounded" @click="play">
          <span class="icon"><mdicon name="shuffle" size="16" /></span>
          <span>Shuffle</span>
        </a>
      </div>
    </template>
    <template #content>
      <p class="heading has-text-centered-mobile">
        {{ albums.total }} albums |
        <a class="has-text-link" @click="open_tracks"
          >{{ track_count }} tracks</a
        >
      </p>
      <list-albums :albums="albums" />
      <modal-dialog-artist
        :show="show_artist_details_modal"
        :artist="modal_artist"
        @close="show_artist_details_modal = false"
      />
    </template>
  </content-with-heading>
  </div>
</template>

<script>
import ContentWithHeading from '@/templates/ContentWithHeading.vue'
import TabsMusic from '@/components/TabsMusic.vue'
import ListAlbums from '@/components/ListAlbums.vue'
import ModalDialogArtist from '@/components/ModalDialogArtist.vue'
import DropdownMenu from '@/components/DropdownMenu.vue'
import IndexList from '@/components/IndexList.vue'
import webapi from '@/webapi'
import * as types from '@/store/mutation_types'
import { bySortName, byYear, GroupByList } from '@/lib/GroupByList'

const dataObject = {
  load: function (to) {
    return Promise.all([
      webapi.library_artist(to.params.artist_id),
      webapi.library_artist_albums(to.params.artist_id)
    ])
  },

  set: function (vm, response) {
    // vm.artist = response[0].data
    vm.albums_list = new GroupByList(response[1].data)

    vm.name = response[0].data.name
    vm.id = response[0].data.id
    vm.artist = response[0].data.items
    vm.albums = response[1].data
  }
}

export default {
  name: 'PageArtist',
  components: {
    ContentWithHeading,
    TabsMusic,
    IndexList,
    ListAlbums,
    ModalDialogArtist,
    DropdownMenu
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
      name: '',
      id: '',
      artist: [],

      albums_list: new GroupByList(),

      // List of group by/sort options for itemsGroupByList
      groupby_options: [
        { name: 'Name', options: bySortName('name_sort') },
        {
          name: 'Release date',
          options: byYear('date_released', {
            direction: 'asc',
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
        album_count: this.albums.items.length,
        track_count: this.track_count,
	//time_added: this.albums.items[0].time_added,
	//data_kind: this.albums.items[0].data_kind,
        uri: this.albums.items.map(a => a.uri).join(',')
      }
    },

    track_count () {
      return this.albums.items.reduce((acc, item) => {
        acc += item.track_count
        return acc
      }, 0)
    },

    albums() {
      const groupBy = this.groupby_options.find(
        (o) => o.name === this.selected_groupby_option_name
      )
      this.albums_list.group(groupBy.options)

      return this.albums_list
    },

    groupby_option_names() {
      return [...this.groupby_options].map((o) => o.name)
    },

    selected_groupby_option_name: {
      get() {
        return this.$store.state.artist_albums_sort
      },
      set(value) {
        this.$store.commit(types.ARTIST_ALBUMS_SORT, value)
      }
    }
  },

  methods: {
    open_tracks: function () {
      this.$router.push({
        path: '/music/artists/' + this.id + '/tracks'
      })
    },

    play: function () {
      webapi.player_play_uri(
        this.albums.items.map((a) => a.uri).join(','),
        true
      )
    }
  }
}
</script>

<style></style>
