<template>
  <div class="fd-page-with-tabs">
    <tabs-music />

    <content-with-heading>
      <template #heading-left>
        <p class="title is-4">Recently added</p>
        <p class="heading">{{ recently_added.total }} tracks</p>
      </template>
      <template #content>
        <list-tracks :tracks="recently_added" :show_group_title="true" @usermark-updated="usermark_upd"/>
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
    return webapi.search({
      type: 'track',
      expression: 'media_kind is music order by time_added desc',
      limit: recentlyAddedLimit
    })
  },

  set: function (vm, response) {
    vm.recently_added = new GroupByList(response.data.tracks)
    vm.recently_added.group(
      byDateSinceToday('time_added', {
        direction: 'desc',
        defaultValue: '0000'
      })
    )
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
    dataObject.load(to).then((response) => {
      dataObject.set(vm, response)
      next()
    })
  },

  data() {
    return {
      recently_added: new GroupByList()
    }
  },

  methods: {
    usermark_upd: function (args, what) {
      this.recently_added.items.find(e => e.id === args.track_id).usermark = args.value
    }
  }
}
</script>

<style>
</style>
