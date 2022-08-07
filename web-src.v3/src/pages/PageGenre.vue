<template>
  <div class="fd-page-with-tabs">
    <tabs-music />

    <index-list :index="albums.indexList"></index-list>

  <div>
    <content-with-heading>
      <template #options>
        <index-button-list :index="albums.indexList" />

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
          {{ genre.name }}
        </p>
      </template>
      <template #heading-right>
        <div class="buttons is-centered">
          <a
            class="button is-small is-light is-rounded"
            @click="show_genre_details_modal = true"
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
          <a class="has-text-link" @click="open_artists"
            >artists | </a
          >
          {{ genre.album_count }} albums |
          <a class="has-text-link" @click="open_tracks"
            >{{ genre.track_count }} tracks</a
          >
          <a class="has-text-link" @click="open_composers"
            >| composers</a
          >
        </p>
        <list-albums :albums="albums" />
        <modal-dialog-genre
          :show="show_genre_details_modal"
          :genre="genre"
          @close="show_genre_details_modal = false"
        />
      </template>
    </content-with-heading>
  </div>
  </div>
</template>

<script>
import ContentWithHeading from '@/templates/ContentWithHeading.vue'
import TabsMusic from '@/components/TabsMusic.vue'
import IndexButtonList from '@/components/IndexButtonList.vue'
import ListAlbums from '@/components/ListAlbums.vue'
import DropdownMenu from '@/components/DropdownMenu.vue'
import ModalDialogGenre from '@/components/ModalDialogGenre.vue'
import IndexList from '@/components/IndexList.vue'
import webapi from '@/webapi'
import * as types from '@/store/mutation_types'
import { bySortName, byYear, GroupByList } from '@/lib/GroupByList'

const dataObject = {
  load: function (to) {
    return Promise.all([
      webapi.library_genre(to.params.genre),
      webapi.library_genre_albums(to.params.genre)
    ])
  },

  set: function (vm, response) {
    vm.genre = response[0].data
    vm.albums_list = new GroupByList(response[1].data.albums)
    vm.albums_list.group(bySortName('name_sort'))
  }
}

export default {
  name: 'PageGenre',
  components: {
    ContentWithHeading,
    TabsMusic,
    IndexList,
    IndexButtonList,
    ListAlbums,
    DropdownMenu,
    ModalDialogGenre
  },

  beforeRouteEnter(to, from, next) {
    dataObject.load(to).then((response) => {
      next((vm) => dataObject.set(vm, response))
    })
  },
  beforeRouteUpdate(to, from, next) {
    if (!this.albums_list.isEmpty()) {
      next()
      return
    }
    const vm = this
    dataObject.load(to).then((response) => {
      dataObject.set(vm, response)
      next()
    })
  },

  data() {
    return {
      genre: {},
      albums_list: new GroupByList(),

      // List of group by/sort options for itemsGroupByList
      groupby_options: [
        { name: 'Name', options: bySortName('name_sort') },
        {
          name: 'Recently added',
          options: byYear('time_added', {
            direction: 'desc',
            defaultValue: '0000'
          })
        },
        {
          name: 'Recently released',
          options: byYear('date_released', {
            direction: 'desc',
            defaultValue: '0000'
          })
        }
      ],

      show_genre_details_modal: false
    }
  },

  computed: {
    albums() {
      const groupBy = this.groupby_options.find(
        (o) => o.name === this.selected_groupby_option_name
      )
      this.albums_list.group(groupBy.options, [ ])

      return this.albums_list
    },

    groupby_option_names() {
      return [...this.groupby_options].map((o) => o.name)
    },

    selected_groupby_option_name: {
      get() {
        return this.$store.state.albums_sort
      },
      set(value) {
        this.$store.commit(types.ALBUMS_SORT, value)
      }
    }
  },

  methods: {
    open_tracks: function () {
      this.show_details_modal = false
      this.$router.push({
        name: 'GenreTracks',
        params: { genre: this.genre.name }
      })
    },

    open_artists: function () {
      this.show_details_modal = false
      this.$router.push({ 
        name: 'GenreArtists',
	params: { genre: this.genre.name }
      })
    },

    open_composers: function () {
      this.show_details_modal = false
      this.$router.push({
        name: 'Composers',
	params: { genre: this.genre.name }
      })
    },

    play: function () {
      webapi.player_play_expression(
        'genre is "' + this.genre.name + '" and media_kind is music',
        true
      )
    }
  }
}
</script>

<style></style>
