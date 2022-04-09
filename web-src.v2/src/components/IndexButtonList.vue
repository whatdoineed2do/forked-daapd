<template>
  <section>
  </section>
</template>

<script>
export default {
  name: 'IndexButtonList',

  props: ['index'],

  computed: {
    filtered_index () {
      const specialChars = '!"#$%&\'()*+,-./:;<=>?@[\\]^`{|}~'
      return this.index.filter(c => !specialChars.includes(c))
    }
  },

  methods: {
    nav: function (id) {
      this.$router.push({ path: this.$router.currentRoute.path + '#index_' + id })
    },

    scroll_to_top: function () {
      window.scrollTo({ top: 0, behavior: 'smooth' })
    }
  }
}
</script>

<style>
</style>
