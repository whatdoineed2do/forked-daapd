<template>
  <div>
    <content-with-heading>
      <template #heading-left>
        <p class="title is-4">Radio</p>
      </template>
      <template #content>
        <p class="heading has-text-centered-mobile">
          {{ tracks.total }} tracks
        </p>
        <list-tracks :tracks="tracks" />
      </template>
    </content-with-heading>
  </div>
</template>

<script>
import ContentWithHeading from '@/templates/ContentWithHeading.vue'
import { GroupByList } from '@/lib/GroupByList'
import ListTracks from '@/components/ListTracks.vue'
import webapi from '@/webapi'

const dataObject = {
  load: function (to) {
    return webapi.library_radio_streams()
  },

  set: function (vm, response) {
    vm.tracks = new GroupByList(response.data.tracks)
  }
}

export default {
  name: 'PageRadioStreams',
  components: { ContentWithHeading, ListTracks },

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
      tracks: new GroupByList()
    }
  }
}
</script>

<style></style>
