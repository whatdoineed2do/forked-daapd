<template>
  <div>
    <a href="#" v-scroll-to="{ el: '#index_' + anchor, offset: -100 }">{{ name }}</a>
  </div>
</template>

<script>
export default {
  name: 'ListIdxNav',

  props: ['name', 'anchor']
}
</script>

<style>
</style>
