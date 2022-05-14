<template>
  <div>
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
          {{ composer.name }}
        </p>
      </template>
      <template #heading-right>
        <div class="buttons is-centered">
          <a
            class="button is-small is-light is-rounded"
            @click="show_composer_details_modal = true"
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
          {{ composer.album_count }} albums |
          <a class="has-text-link" @click="open_tracks"
            >{{ composer.track_count }} tracks</a
          >
        </p>
        <list-albums :albums="albums_list" :hide_group_title="true" />

        <modal-dialog-composer
          :show="show_composer_details_modal"
          :composer="composer"
          @close="show_composer_details_modal = false"
        />
      </template>
    </content-with-heading>
  </div>
</template>

<script>
import ContentWithHeading from '@/templates/ContentWithHeading.vue'
import TabsMusic from '@/components/TabsMusic.vue'
import ListAlbums from '@/components/ListAlbums.vue'
import ModalDialogComposer from '@/components/ModalDialogComposer.vue'
import IndexList from '@/components/IndexList.vue'
import webapi from '@/webapi'
import { bySortName, byYear, GroupByList } from '@/lib/GroupByList'

const dataObject = {
  load: function (to) {
    return Promise.all([
      webapi.library_composer(to.params.composer),
      webapi.library_composer_albums(to.params.composer)
    ])
  },

  set: function (vm, response) {
    vm.composer = response[0].data
    vm.albums_list = new GroupByList(response[1].data.albums)
  }
}

export default {
  name: 'PageComposer',
  components: {
    ContentWithHeading,
    TabsMusic,
    IndexList,
    ListAlbums,
    ModalDialogComposer
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
      composer: {},
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

      show_composer_details_modal: false
    }
  },

  computed: {
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
        name: 'ComposerTracks',
        params: { composer: this.composer.name }
      })
    },

    play: function () {
      webapi.player_play_expression(
        'composer is "' + this.composer.name + '" and media_kind is music',
        true
      )
    }
  }
}
</script>

<style></style>
