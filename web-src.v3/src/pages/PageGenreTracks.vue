<template>
  <div class="fd-page-with-tabs">
    <tabs-music />

    <index-list :index="index_list"></index-list>

  <div>
    <content-with-heading>
      <template #options>
        <index-button-list :index="index_list" />
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
          <a class="has-text-link" @click="open_genre"
            >{{ genre.album_count }} albums</a
          >
          | {{ genre.track_count }} tracks
        </p>
        <list-tracks :tracks="tracks.items" :expression="expression" />
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
import IndexList from '@/components/IndexList.vue'
import IndexButtonList from '@/components/IndexButtonList.vue'
import ListTracks from '@/components/ListTracks.vue'
import ModalDialogGenre from '@/components/ModalDialogGenre.vue'
import webapi from '@/webapi'

const dataObject = {
  load: function (to) {
    return Promise.all([
      webapi.library_genre(to.params.genre),
      webapi.library_genre_tracks(to.params.genre)
    ])
  },

  set: function (vm, response) {
    vm.genre = response[0].data
    vm.tracks = response[1].data.tracks
  }
}

export default {
  name: 'PageGenreTracks',
  components: {
    ContentWithHeading,
    TabsMusic,
    IndexList,
    ListTracks,
    IndexButtonList,
    ModalDialogGenre
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
      tracks: { items: [] },
      genre: '',

      show_genre_details_modal: false
    }
  },

  computed: {
    index_list() {
      return [
        ...new Set(
          this.tracks.items.map((track) =>
            track.title_sort.charAt(0).toUpperCase()
          )
        )
      ]
    },

    expression() {
      return 'genre is "' + this.genre.name + '" and media_kind is music'
    }
  },

  methods: {
    open_genre: function () {
      this.show_details_modal = false
      this.$router.push({ name: 'Genre', params: { genre: this.genre.name } })
    },

    play: function () {
      webapi.player_play_expression(this.expression, true)
    }
  }
}
</script>

<style></style>