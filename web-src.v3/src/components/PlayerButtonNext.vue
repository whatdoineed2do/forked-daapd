<template>
  <a :disabled="disabled" @click="play_next">
    <span class="icon"><mdicon name="skip-forward" :size="icon_size" /></span>
  </a>
</template>

<script>
import webapi from '@/webapi'

export default {
  name: 'PlayerButtonNext',

  props: {
    icon_size: {
      type: Number,
      default: 16
    }
  },

  computed: {
    disabled() {
      return !this.$store.state.queue || this.$store.state.queue.count <= 0
    }
  },

  methods: {
    play_next: function () {
      if (this.disabled) {
        return
      }

      webapi.player_next()
    }
  }
}
</script>

<style></style>
