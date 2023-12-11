<template>
  <a :disabled="disabled" @click="play_stop">
    <span class="icon"><mdicon :name="stop" :size="icon_size" /></span>
  </a>
</template>

<script>
import webapi from '@/webapi'

export default {
  name: 'PlayerButtonStop',

  props: {
    icon_size: {
      type: Number,
      default: 16
    },
    show_disabled_message: Boolean
  },

  computed: {
    is_playing() {
      return this.$store.state.player.state === 'play'
    },

    is_stop_allowed() {
      return (
        this.$store.getters.now_playing &&
        this.$store.getters.now_playing.data_kind !== 'pipe'
      )
    },

    disabled() {
      return !this.$store.state.queue || this.$store.state.queue.count <= 0
    },

    icon_name() {
      if (!this.is_playing) {
        return 'play'
      } else if (this.is_stop_allowed) {
        return 'pause'
      }
      return 'stop'
    }
  },

  methods: {
    play_stop: function () {
      if (this.disabled) {
        if (this.show_disabled_message) {
          this.$store.dispatch('add_notification', {
            text: 'Queue is empty',
            type: 'info',
            topic: 'connection',
            timeout: 2000
          })
        }
        return
      }

      if (this.is_playing && this.is_stop_allowed) {
        webapi.player_pause()
      } else if (this.is_playing && !this.is_stop_allowed) {
        webapi.player_stop()
      } else {
        webapi.player_play()
      }
    }
  }
}
</script>

<style></style>
