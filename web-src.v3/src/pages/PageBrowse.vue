<template>
  <div class="fd-page-with-tabs">
    <tabs-music />

    <!-- Recently added -->
    <content-with-heading>
      <template #heading-left>
        <p class="title is-4">Recently added</p>
        <p class="heading">albums</p>
      </template>
      <template #content>
        <list-albums :albums="recently_added" />
      </template>
      <template #footer>
        <nav class="level">
          <p class="level-item">
            <a
              class="button is-light is-small is-rounded"
              @click="open_browse('recently_added')"
              >Show more</a
            >
          </p>
        </nav>
      </template>
    </content-with-heading>

    <!-- Recently added Tracks -->
    <content-with-heading>
      <template #heading-left>
        <p class="title is-4">Recently added</p>
        <p class="heading">Tracks</p>
      </template>
      <template #content>
        <list-tracks :tracks="recently_added_tracks" @usermark-updated="usermark_upd_added" />
      </template>
      <template #footer>
        <nav class="level">
          <p class="level-item">
            <a
              class="button is-light is-small is-rounded"
              @click="open_browse('recently_added_tracks')"
              >Show more</a
            >
          </p>
        </nav>
      </template>
    </content-with-heading>

    <!-- Recently played -->
    <content-with-heading>
      <template #heading-left>
        <p class="title is-4">Recently played</p>
        <p class="heading">tracks</p>
      </template>
      <template #content>
        <list-tracks :tracks="recently_played" @usermark-updated="usermark_upd_played" />
      </template>
      <template #footer>
        <nav class="level">
          <p class="level-item">
            <a
              class="button is-light is-small is-rounded"
              @click="open_browse('recently_played')"
              >Show more</a
            >
          </p>
        </nav>
      </template>
    </content-with-heading>
  </div>
</template>

<script>
import ContentWithHeading from '@/templates/ContentWithHeading.vue'
import { GroupByList } from '@/lib/GroupByList'
import TabsMusic from '@/components/TabsMusic.vue'
import ListAlbums from '@/components/ListAlbums.vue'
import ListTracks from '@/components/ListTracks.vue'
import webapi from '@/webapi'

const dataObject = {
  load: function (to) {
    return Promise.all([
      webapi.search({
        type: 'album',
        expression:
          'time_added after 8 weeks ago and media_kind is music having track_count > 3 order by time_added desc',
        limit: 3
      }),
      webapi.search({
        type: 'track',
        expression:
          'time_played after 8 weeks ago and media_kind is music order by time_played desc',
        limit: 3
      }),
      webapi.search({
        type: 'track',
        expression:
          'time_added after 8 weeks ago and media_kind is music order by time_added desc',
        limit: 3
      })
    ])
  },

  set: function (vm, response) {
    vm.recently_added = new GroupByList(response[0].data.albums)
    vm.recently_played = new GroupByList(response[1].data.tracks)
    vm.recently_added_tracks = new GroupByList(response[2].data.tracks)
  }
}

export default {
  name: 'PageBrowse',
  components: { ContentWithHeading, TabsMusic, ListAlbums, ListTracks },

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
      recently_added: [],
      recently_played: { items: [] },
      recently_added_tracks: { items: [] },

      show_track_details_modal: false,
      selected_track: {}
    }
  },

  methods: {
    open_browse: function (type) {
      this.$router.push({ path: '/music/browse/' + type })
    },

    usermark_upd_added: function (args) {
      this.usermark_upd(this.recently_added_tracks.items, args)
    },
    usermark_upd_played: function (args) {
      this.usermark_upd(this.recently_played.items, args)
    },

    usermark_upd: function (what, args) {
      what.find(e => e.id === args.track_id).usermark = args.value
    }
  }
}
</script>

<style></style>
