<template>
  <a @click="play_stop" :disabled="disabled">
    <span class="icon"><i class="mdi mdi-stop" :class="icon_style1"></i></span>
  </a>
</template>

<script>
import webapi from '@/webapi'

export default {
  name: 'PlayerButtonStop',

  props: {
    icon_style: String
  },

  computed: {
    disabled () {
      return !this.$store.state.queue || this.$store.state.queue.count <= 0
    },
    icon_style1 () {
      return this.icon_style + ' ' + (this.$store.state.player.state === 'stop' ? 'has-text-grey-light' : 'has-text-black')
    }
  },

  methods: {
    play_stop: function () {
      webapi.player_stop()
    }
  }
}
</script>

<style>
</style>
