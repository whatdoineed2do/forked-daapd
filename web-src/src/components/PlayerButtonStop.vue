<template>
  <a @click="play_stop" :disabled="disabled">
    <span class="icon"><i class="mdi mdi-stop" :class="icon_style"></i></span>
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
