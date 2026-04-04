<template>
  <div class="fd-page-with-tabs">
    <tabs-music />

    <content-with-heading>
      <template #heading-left>
        <p class="title is-4">Recently added</p>
        <p class="heading">{{ recently_added.count }} tracks</p>
      </template>
      <template #content>
        <list-tracks-w-headings
          :tracks="recently_added"
          @usermark-updated="usermark_upd"
        />
      </template>
      <template #footer>
        <nav class="level">
          <p class="level-item">
            <a class="button is-light is-small is-rounded" @click="load_more()"
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
import TabsMusic from '@/components/TabsMusic.vue'
import ListTracksWHeadings from '@/components/ListTracksWHeadings.vue'
import ModalDialogTracks from '@/components/ModalDialogTracks.vue'
import webapi from '@/webapi'
import store from '@/store'
import { byDateSinceToday, GroupByList } from '@/lib/GroupByList'

const dataObject = {
  load: function (page = 1) {
    const perPage = store.getters.settings_option_recently_added_limit
    const offset = perPage * (page - 1)
    console.debug('[PageBrowseRecentlyAddedTracks] dataObject.load', { page, perPage, offset })
    return webapi.search({
      type: 'track',
      expression: 'media_kind is music order by time_added desc',
      limit: perPage,
      offset: offset
    })
  },

  set: function (vm, response, page = 1) {
    try {
      // Handle both shapes: response.data.tracks may be object or array, or response.data may be an array
      let tracksObj
      if (Array.isArray(response.data)) {
        tracksObj = { items: response.data, total: response.data.length, offset: 0, limit: -1 }
      } else if (response.data && Array.isArray(response.data.tracks)) {
        tracksObj = response.data.tracks
      } else if (response.data && response.data.tracks && Array.isArray(response.data.tracks.items)) {
        tracksObj = response.data.tracks
      } else if (response.data && response.data.items && Array.isArray(response.data.items)) {
        // sometimes API returns { items: [...] }
        tracksObj = { items: response.data.items, total: response.data.total || response.data.items.length, offset: response.data.offset || 0, limit: response.data.limit || -1 }
      } else {
        tracksObj = { items: [], total: 0, offset: 0, limit: -1 }
      }

      console.debug('[PageBrowseRecentlyAddedTracks] dataObject.set response', { page, tracksCount: (tracksObj.items || []).length, total: tracksObj.total, offset: tracksObj.offset, limit: tracksObj.limit })

      if (page > 1 && vm.recently_added && vm.recently_added.items) {
        // append and deduplicate by id — mutate existing GroupByList to preserve instance
        const existing = vm.recently_added.items || []
        const incoming = tracksObj.items || []

        const existingIds = new Set(existing.map((t) => t.id))
        const newItems = incoming.filter((t) => !existingIds.has(t.id))

        if (newItems.length > 0) {
          // create a new GroupByList from combined items (avoid mutating custom instance)
          const combined = existing.concat(newItems)
          vm.recently_added = new GroupByList({
            items: combined,
            total: tracksObj.total || combined.length,
            offset: 0,
            limit: tracksObj.limit || -1
          })
        }

        // regroup the list on the new GroupByList instance
        vm.recently_added.group(
          byDateSinceToday('time_added', {
            direction: 'desc',
            defaultValue: '0000'
          })
        )
      } else {
        vm.recently_added = new GroupByList(tracksObj)

        // ensure grouping runs and itemsByGroup is populated for the initial set
        vm.recently_added.group(
          byDateSinceToday('time_added', {
            direction: 'desc',
            defaultValue: '0000'
          })
        )
      }

      /*
      console.debug('[PageBrowseRecentlyAddedTracks] dataObject.set completed', {
        currentCount: vm.recently_added.count,
        indexList: vm.recently_added.indexList,
        groups: Object.keys(vm.recently_added.itemsByGroup || {}).length
      })
       */
    } catch (err) {
      console.error('[PageBrowseRecentlyAddedTracks] dataObject.set error', err)
      throw err
    }
  }
}

export default {
  name: 'PageBrowseType',
  components: {
    ContentWithHeading,
    TabsMusic,
    ListTracksWHeadings,
    ModalDialogTracks
  },

  beforeRouteEnter(to, from, next) {
    dataObject.load(1).then((response) => {
      next((vm) => dataObject.set(vm, response, 1))
    }).catch((err) => {
      console.error('[PageBrowseRecentlyAddedTracks] beforeRouteEnter load error', err)
      next()
    })
  },

  beforeRouteUpdate(to, from, next) {
    if (!this.recently_added.isEmpty()) {
      next()
      return
    }
    const vm = this
    dataObject.load(this.pagnation).then((response) => {
      dataObject.set(vm, response, this.pagnation)
      next()
    })
  },

  data() {
    return {
      recently_added: new GroupByList(),
      pagnation: 1
    }
  },

  methods: {
    usermark_upd: function (args) {
      this.recently_added.items.find((e) => e.id === args.track_id).usermark =
        args.value
    },

    load_more: function () {
      this.pagnation = (this.pagnation || 1) + 1
      const page = this.pagnation
      const vm = this
      //console.debug('[PageBrowseRecentlyAddedTracks] load_more requested', { page })
      dataObject.load(page).then((response) => {
        //console.debug('[PageBrowseRecentlyAddedTracks] load_more response', response)
        try {
          // quick sanity checks to help debugging
          const respItems = (response && response.data && (response.data.tracks && response.data.tracks.items)) || (response && response.data && response.data.items) || (Array.isArray(response && response.data) && response.data)
          if (!respItems || respItems.length === 0) {
            //console.warn('[PageBrowseRecentlyAddedTracks] load_more returned no items', { page, respItems })
            // revert pagnation increment
            this.pagnation = Math.max(1, this.pagnation - 1)
            try {
              store.dispatch('add_notification', { text: 'No additional tracks returned from server', type: 'info', timeout: 2000 })
            } catch (e) {
              console.debug('[PageBrowseRecentlyAddedTracks] notification dispatch failed', e)
            }
            return
          }

          // check for duplicates before applying — if all incoming ids already exist, revert pagnation
          const existingIds = new Set((vm.recently_added && vm.recently_added.items ? vm.recently_added.items.map(t => t.id) : []))
          const incomingIds = Array.isArray(respItems) ? respItems.map(t => t.id) : []
          const hasNew = incomingIds.some(id => !existingIds.has(id))
          if (!hasNew) {
            //console.warn('[PageBrowseRecentlyAddedTracks] load_more incoming items are duplicates', { page, existingCount: existingIds.size, incomingCount: incomingIds.length })
            this.pagnation = Math.max(1, this.pagnation - 1)
            try { store.dispatch('add_notification', { text: 'No new tracks to append', type: 'info', timeout: 1500 }) } catch (e) { console.debug('[PageBrowseRecentlyAddedTracks] notification dispatch failed', e) }
            return
          }

          dataObject.set(vm, response, page)
          console.debug('[PageBrowseRecentlyAddedTracks] load_more done', { page, currentCount: vm.recently_added.count })
        } catch (err) {
          console.error('[PageBrowseRecentlyAddedTracks] load_more set error', err)
          this.pagnation = Math.max(1, this.pagnation - 1)
        }
      }).catch((err) => {
        console.error('[PageBrowseRecentlyAddedTracks] load_more load error', err)
        this.pagnation = Math.max(1, this.pagnation - 1)
      })
    }
  }
}
</script>

<style></style>
