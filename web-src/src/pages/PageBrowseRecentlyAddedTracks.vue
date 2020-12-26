<template>
  <div>
    <tabs-music></tabs-music>

    <content-with-heading>
      <template slot="heading-left">
        <p class="title is-4">Recently added</p>
        <p class="heading">{{ recently_added }} tracks</p>
      </template>
    </content-with-heading>

    <content-with-heading v-if="show_recent_today && recently_added_today.items.length">
      <template slot="heading-left">
        <p class="title is-6">Today</p>
        <p class="heading">{{ recently_added_today.items.length }} tracks</p>
      </template>
      <template slot="heading-right">
        <div class="buttons is-centered">
          <a class="button is-small is-light is-rounded" @click="show_modal_today = true">
            <span class="icon"><i class="mdi mdi-dots-horizontal mdi-18px"></i></span>
          </a>
        </div>
      </template>
      <template slot="content">
        <list-tracks :tracks="recently_added_today.items"></list-tracks>
        <modal-dialog-tracks :show="show_modal_today" title='Recently Added - Today' :tracks="recently_added_today" @close="show_modal_today = false" />
      </template>
    </content-with-heading>

    <content-with-heading v-if="show_recent_week && recently_added_week.items.length">
      <template slot="heading-left">
        <p class="title is-6">This Week</p>
        <p class="heading">{{ recently_added_week.items.length }} tracks</p>
      </template>
      <template slot="heading-right">
        <div class="buttons is-centered">
          <a class="button is-small is-light is-rounded" @click="show_modal_week = true">
            <span class="icon"><i class="mdi mdi-dots-horizontal mdi-18px"></i></span>
          </a>
        </div>
      </template>
      <template slot="content">
        <list-tracks :tracks="recently_added_week.items"></list-tracks>
        <modal-dialog-tracks :show="show_modal_week" title='Recently Added - Past Week' :tracks="recently_added_week" @close="show_modal_week = false" />
      </template>
    </content-with-heading>

    <content-with-heading v-if="show_recent_month && recently_added_month.items.length">
      <template slot="heading-left">
        <p class="title is-6">This Month </p>
        <p class="heading">{{ recently_added_month.items.length }} tracks</p>
      </template>
      <template slot="heading-right">
        <div class="buttons is-centered">
          <a class="button is-small is-light is-rounded" @click="show_modal_month = true">
            <span class="icon"><i class="mdi mdi-dots-horizontal mdi-18px"></i></span>
          </a>
        </div>
      </template>
      <template slot="content">
        <list-tracks :tracks="recently_added_month.items"></list-tracks>
        <modal-dialog-tracks :show="show_modal_month" title='Recently Added - Past Month' :tracks="recently_added_month" @close="show_modal_month = false" />
      </template>
    </content-with-heading>

    <content-with-heading v-if="show_recent_older && recently_added_older.items.length">
      <template slot="heading-left">
        <p class="title is-6">Older</p>
        <p class="heading">{{ recently_added_older.items.length }} tracks</p>
      </template>
      <template slot="heading-right">
        <div class="buttons is-centered">
          <a class="button is-small is-light is-rounded" @click="show_modal_older = true">
            <span class="icon"><i class="mdi mdi-dots-horizontal mdi-18px"></i></span>
          </a>
        </div>
      </template>
      <template slot="content">
        <list-tracks :tracks="recently_added_older.items"></list-tracks>
        <modal-dialog-tracks :show="show_modal_older" title='Recently Added - Older than Month' :tracks="recently_added_older" @close="show_modal_older = false" />
      </template>
    </content-with-heading>
  </div>
</template>

<script>
import { LoadDataBeforeEnterMixin } from './mixin'
import ContentWithHeading from '@/templates/ContentWithHeading'
import TabsMusic from '@/components/TabsMusic'
import ListTracks from '@/components/ListTracks'
import ModalDialogTracks from '@/components/ModalDialogTracks'
import webapi from '@/webapi'
import store from '@/store'

const browseData = {
  load: function (to) {
    const recentlyAddedLimit = store.getters.settings_option_recently_added_limit
    return Promise.all([
      webapi.search({
        type: 'track',
        expression: 'time_added after today and media_kind is music order by time_added desc',
        limit: recentlyAddedLimit
      }),
      webapi.search({
        type: 'track',
        expression: 'time_added after this week and media_kind is music order by time_added desc',
        limit: recentlyAddedLimit
      }),
      webapi.search({
        type: 'track',
        expression: 'time_added after last month and media_kind is music order by time_added desc',
        limit: recentlyAddedLimit
      })
    ])
  },

  set: function (vm, response) {
    vm.recently_added_today = response[0].data.tracks
    vm.recently_added_week = response[1].data.tracks
    vm.recently_added_month = response[2].data.tracks
  }
}

export default {
  name: 'PageBrowseType',
  mixins: [LoadDataBeforeEnterMixin(browseData)],
  components: { ContentWithHeading, TabsMusic, ListTracks, ModalDialogTracks },

  data () {
    return {
      recently_added_today: { items: [] },
      recently_added_week: { items: [] },
      recently_added_month: { items: [] },
      recently_added_older: { items: [] },

      show_modal_today: false,
      show_modal_week: false,
      show_modal_month: false,
      show_modal_older: false
    }
  },

  mounted () {
    const more = store.getters.settings_option_recently_added_limit - this.recently_added_month.items.length
    if (more) {
      webapi.search({
        type: 'track',
        expression: 'time_added before last month and media_kind is music order by time_added desc',
        limit: more
      }).then(({ data }) => {
        this.recently_added_older = data.tracks
      })
    }
  },

  computed: {
    show_recent_today () {
      return this.recently_added_today.items.length > 0
    },
    show_recent_week () {
      return this.recently_added_week.items.length > 0
    },
    show_recent_month () {
      return this.recently_added_month.items.length > 0
    },
    show_recent_older () {
      return this.recently_added_older.items.length > 0
    },
    recently_added () {
      return this.recently_added_older.items.length + this.recently_added_month.items.length
    }
  }
}
</script>

<style>
</style>
