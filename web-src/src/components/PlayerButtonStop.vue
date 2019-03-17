<template>
  <a v-on:click="play_stop">
    <span class="icon"><i class="mdi mdi-stop"></i></span>
  </a>
</template>

<script>
import webapi from '@/webapi'

export default {
  name: 'PlayerButtonStop',

  methods: {
    play_stop: function () {
      webapi.player_stop()
    }
  }
}
</script>

<style>
</style>
