<template>
  <div>
    <tabs-music></tabs-music>
    <section class="section fd-tabs-bottom-section">
      <template>
        <div class="container" v-if="links.length > 1">
          <div class="columns is-centered">
            <div class="column is-three-quarters">
              <div class="tabs is-centered is-small">
                <ul>
                  <tab-idx-nav-item v-for="link in links" :key="link.n" :link="link"></tab-idx-nav-item>
                </ul>
              </div>
            </div>
          </div>
        </div>
      </template>
    </section>

    <content-with-heading>
      <template slot="heading-left">
        <p class="title is-4">{{ genre }}</p>
        <p class="heading">{{ tracks.total }} tracks</p>
      </template>
      <template slot="heading-right">
        <a class="button is-small is-dark is-rounded" @click="play">
          <span class="icon"><i class="mdi mdi-shuffle"></i></span> <span>Shuffle</span>
        </a>
      </template>
      <template slot="content">
        <p class="heading has-text-centered-mobile"><a class="has-text-link" @click="open_genre">albums</a> | tracks</p>
        <list-item-track v-for="(track, index) in tracks.items" :key="track.id" :track="track" :position="index" :context_uri="tracks.items.map(a => a.uri).join(',')" :links="links"></list-item-track>
      </template>
    </content-with-heading>
  </div>
</template>

<script>
import { LoadDataBeforeEnterMixin } from './mixin'
import ContentWithHeading from '@/templates/ContentWithHeading'
import TabsMusic from '@/components/TabsMusic'
import ListItemTrack from '@/components/ListItemTrack'
import TabIdxNavItem from '@/components/TabsIdxNav'
import webapi from '@/webapi'

const tracksData = {
  load: function (to) {
    return webapi.library_genre_tracks(to.params.genre)
  },

  set: function (vm, response) {
    vm.genre = vm.$route.params.genre
    vm.tracks = response.data.tracks

    var li = 0
    var v = null
    var i
    for (i = 0; i < vm.tracks.items.length; i++) {
      var n = vm.tracks.items[i].title.charAt(0).toUpperCase()
      if (n !== v) {
        var obj = {}
        obj.n = n
        obj.a = 'idx_nav_' + li
        vm.links.push(obj)
        li++
        v = n
      }
    }
  }
}

export default {
  name: 'PageGenreTracks',
  mixins: [ LoadDataBeforeEnterMixin(tracksData) ],
  components: { ContentWithHeading, TabsMusic, ListItemTrack, TabIdxNavItem },

  data () {
    return {
      tracks: {},
      genre: '',
      links: []
    }
  },

  methods: {
    open_genre: function () {
      this.show_details_modal = false
      this.$router.push({ path: '/music/genres/' + this.genre })
    },

    play: function () {
      webapi.player_play_uri(this.tracks.items.map(a => a.uri).join(','), true)
    }
  }
}
</script>

<style>
</style>
