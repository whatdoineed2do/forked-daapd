<template>
  <div>
    <a href="#" v-scroll-to="{ el: filtered_anchor, offset: -100 }">{{ name }}</a>
  </div>
</template>

<script>
export default {
  name: 'ListIdxNav',

  props: ['name', 'anchor'],

  computed: {
    filtered_anchor () {
      try {
	  return '#index_' + this.anchor.replace(' ', '')
      }
      catch (err) {
	  return '#index_' + this.anchor
      }
    }
  }
}
</script>

<style>
</style>
