<template>
  <div class="fd-page-with-tabs">
    <tabs-music />

    <content-with-heading>
      <template #heading-left>
        <p class="title is-4">Recently added</p>
        <p class="heading">{{ recently_added }} tracks</p>
      </template>
    </content-with-heading>

    <content-with-heading v-if="show_recent_today && recently_added_today.items.length">
      <template #heading-left>
        <p class="title is-6">Today</p>
        <p class="heading">{{ recently_added_today.items.length }} tracks</p>
      </template>
      <template #heading-right>
        <div class="buttons is-centered">
          <a class="button is-small is-light is-rounded" @click="show_modal_today = true">
            <span class="icon"><i class="mdi mdi-dots-horizontal mdi-18px"></i></span>
          </a>
        </div>
      </template>
      <template #content>
        <list-tracks :tracks="recently_added_today.items" @usermark-updated="usermark_upd_today"></list-tracks>
        <modal-dialog-tracks :show="show_modal_today" title='Recently Added - Today' :tracks="recently_added_today" @close="show_modal_today = false" />
      </template>
    </content-with-heading>

    <content-with-heading v-if="show_recent_week && recently_added_week.items.length">
      <template #heading-left>
        <p class="title is-6">This Week</p>
        <p class="heading">{{ recently_added_week.items.length }} tracks</p>
      </template>
      <template #heading-right>
        <div class="buttons is-centered">
          <a class="button is-small is-light is-rounded" @click="show_modal_week = true">
            <span class="icon"><i class="mdi mdi-dots-horizontal mdi-18px"></i></span>
          </a>
        </div>
      </template>
      <template #content>
        <list-tracks :tracks="recently_added_week.items" @usermark-updated="usermark_upd_week"></list-tracks>
        <modal-dialog-tracks :show="show_modal_week" title='Recently Added - Past Week' :tracks="recently_added_week" @close="show_modal_week = false" />
      </template>
    </content-with-heading>

    <content-with-heading v-if="show_recent_month && recently_added_month.items.length">
      <template #heading-left>
        <p class="title is-6">This Month </p>
        <p class="heading">{{ recently_added_month.items.length }} tracks</p>
      </template>
      <template #heading-right>
        <div class="buttons is-centered">
          <a class="button is-small is-light is-rounded" @click="show_modal_month = true">
            <span class="icon"><i class="mdi mdi-dots-horizontal mdi-18px"></i></span>
          </a>
        </div>
      </template>
      <template #content>
        <list-tracks :tracks="recently_added_month.items" @usermark-updated="usermark_upd_month"></list-tracks>
        <modal-dialog-tracks :show="show_modal_month" title='Recently Added - Past Month' :tracks="recently_added_month" @close="show_modal_month = false" />
      </template>
    </content-with-heading>

    <content-with-heading v-if="show_recent_older && recently_added_older.items.length">
      <template #heading-left>
        <p class="title is-6">Older</p>
        <p class="heading">{{ recently_added_older.items.length }} tracks</p>
      </template>
      <template #heading-right>
        <div class="buttons is-centered">
          <a class="button is-small is-light is-rounded" @click="show_modal_older = true">
            <span class="icon"><i class="mdi mdi-dots-horizontal mdi-18px"></i></span>
          </a>
        </div>
      </template>
      <template #content>
        <list-tracks :tracks="recently_added_older.items" @usermark-updated="usermark_upd_older"></list-tracks>
        <modal-dialog-tracks :show="show_modal_older" title='Recently Added - Older than Month' :tracks="recently_added_older" @close="show_modal_older = false" />
      </template>
    </content-with-heading>
  </div>
</template>

<script>
import ContentWithHeading from '@/templates/ContentWithHeading.vue'
import TabsMusic from '@/components/TabsMusic.vue'
import ListTracks from '@/components/ListTracks.vue'
import ModalDialogTracks from '@/components/ModalDialogTracks.vue'
import webapi from '@/webapi'
import store from '@/store'
import { byDateSinceToday, GroupByList } from '@/lib/GroupByList'

const dataObject = {
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
        expression: 'time_added after this week and time_added before today and media_kind is music order by time_added desc',
        limit: recentlyAddedLimit
      }),
      webapi.search({
        type: 'track',
        expression: 'time_added after last month and time_added before this week and media_kind is music order by time_added desc',
        limit: recentlyAddedLimit
      })
    ])
  },

  set: function (vm, response) {
    let N = store.getters.settings_option_recently_added_limit

    vm.recently_added_today = response[0].data.tracks
    if (vm.recently_added_today.length > N) {
      vm.recently_added_today.length = N
      N = 0
    } else {
      N -= vm.recently_added_today.length
    }

    vm.recently_added_week = response[1].data.tracks
    if (vm.recently_added_week.length > N) {
      vm.recently_added_week.length = N
      N = 0
    } else {
      N -= vm.recently_added_week.length
    }

    vm.recently_added_month = response[2].data.tracks
    if (vm.recently_added_month.length > N) {
      vm.recently_added_month.length = N
    }
  }
}

export default {
  name: 'PageBrowseType',
  components: { ContentWithHeading, TabsMusic, ListTracks, ModalDialogTracks },

  beforeRouteEnter(to, from, next) {
    dataObject.load(to).then((response) => {
      next((vm) => dataObject.set(vm, response))
    })
  },

  beforeRouteUpdate(to, from, next) {
    if (!this.recently_added.isEmpty()) {
      next()
      return
    }
    const vm = this
    dataObject.load(vm, to).then((response) => {
      dataObject.set(vm, response)
      next()
    })
  },


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

  mounted: function () {
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

  methods: {
    usermark_upd_today: function (args) {
      this.usermark_upd(this.recently_added_today.items, args)
    },
    usermark_upd_week: function (args) {
      this.usermark_upd(this.recently_added_week.items, args)
    },
    usermark_upd_month: function (args) {
      this.usermark_upd(this.recently_added_month.items, args)
    },
    usermark_upd_older: function (args) {
      this.usermark_upd(this.recently_added_older.items, args)
    },

    usermark_upd: function (what, args) {
      what.find(e => e.id === args.track_id).usermark = args.value
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
      return this.recently_added_older.items.length + this.recently_added_month.items.length + this.recently_added_week.items.length + this.recently_added_today.items.length
    }
  }
}
</script>

<style>
</style>
