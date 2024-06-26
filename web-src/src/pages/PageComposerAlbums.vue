<template>
  <div>
    <content-with-heading>
      <template #heading-left>
        <p class="title is-4" v-text="composer.name" />
      </template>
      <template #heading-right>
        <div class="buttons is-centered">
          <a
            class="button is-small is-light is-rounded"
            @click="show_details_modal = true"
          >
            <mdicon class="icon" name="dots-horizontal" size="16" />
          </a>
          <a class="button is-small is-dark is-rounded" @click="play">
            <mdicon class="icon" name="shuffle" size="16" />
            <span v-text="$t('page.composer.shuffle')" />
          </a>
        </div>
      </template>
      <template #content>
        <p class="heading has-text-centered-mobile">
          <span
            v-text="
              $t('page.composer.album-count', { count: composer.album_count })
            "
          />
          <span>&nbsp;|&nbsp;</span>
          <a
            class="has-text-link"
            @click="open_tracks"
            v-text="
              $t('page.composer.track-count', { count: composer.track_count })
            "
          />
        </p>
        <list-albums :items="albums" />
        <modal-dialog-composer
          :item="composer"
          :show="show_details_modal"
          @close="show_details_modal = false"
        />
      </template>
    </content-with-heading>
  </div>
</template>

<script>
import ContentWithHeading from '@/templates/ContentWithHeading.vue'
import { GroupedList } from '@/lib/GroupedList'
import ListAlbums from '@/components/ListAlbums.vue'
import ModalDialogComposer from '@/components/ModalDialogComposer.vue'
import webapi from '@/webapi'

const dataObject = {
  load(to) {
    return Promise.all([
      webapi.library_composer(to.params.name),
      webapi.library_composer_albums(to.params.name)
    ])
  },

  set(vm, response) {
    vm.composer = response[0].data
    vm.albums = new GroupedList(response[1].data.albums)
  }
}

export default {
  name: 'PageComposerAlbums',
  components: {
    ContentWithHeading,
    ListAlbums,
    ModalDialogComposer
  },

  beforeRouteEnter(to, from, next) {
    dataObject.load(to).then((response) => {
      next((vm) => dataObject.set(vm, response))
    })
  },

  data() {
    return {
      albums: new GroupedList(),
      composer: {},
      show_details_modal: false
    }
  },

  methods: {
    open_tracks() {
      this.$router.push({
        name: 'music-composer-tracks',
        params: { name: this.composer.name }
      })
    },
    play() {
      webapi.player_play_expression(
        `composer is "${this.composer.name}" and media_kind is music`,
        true
      )
    }
  }
}
</script>

<style></style>
