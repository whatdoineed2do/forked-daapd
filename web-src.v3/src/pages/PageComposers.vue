<template>
  <div>
    <tabs-music />

    <index-list :index="composers.indexList"></index-list>

    <content-with-heading>
      <template #options>
        <index-button-list :index="composers.indexList" />
      </template>
      <template #heading-left>
        <p class="title is-4">{{ heading }}</p>
        <p class="heading">{{ composers.total }} composers</p>
      </template>
      <template #content>
        <list-composers :composers="composers" />
      </template>
    </content-with-heading>
  </div>
</template>

<script>
import ContentWithHeading from '@/templates/ContentWithHeading.vue'
import TabsMusic from '@/components/TabsMusic.vue'
import IndexButtonList from '@/components/IndexButtonList.vue'
import ListComposers from '@/components/ListComposers.vue'
import IndexList from '@/components/IndexList.vue'
import webapi from '@/webapi'
import { byName, GroupByList } from '@/lib/GroupByList'

const dataObject = {
  load: function (to) {
    if (to.params.genre) {
      return webapi.library_genre_composers(to.params.genre)
    }
    return webapi.library_composers('music')
  },

  set: function (vm, response) {
    if (response.data.composers) {
      vm.composers = new GroupByList(response.data.composers)
      vm.heading = vm.$route.params.genre
    } else {
      vm.composers = new GroupByList(response.data)
      vm.heading = 'Composers'
    }
    vm.composers.group(byName('name_sort'))
  }
}

export default {
  name: 'PageComposers',
  components: { ContentWithHeading, TabsMusic, IndexList, IndexButtonList, ListComposers },

  beforeRouteEnter(to, from, next) {
    dataObject.load(to).then((response) => {
      next((vm) => dataObject.set(vm, response))
    })
  },

  beforeRouteUpdate(to, from, next) {
    if (!this.composers.isEmpty()) {
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
      heading: '',
      composers: new GroupByList()
    }
  },

  methods: {}
}
</script>

<style></style>
