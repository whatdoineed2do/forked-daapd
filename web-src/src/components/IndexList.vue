<template>
  <section class="section fd-tabs-bottom-section">
    <div class="container">
      <div class="columns is-centered">
        <div class="column is-four-fifths">
          <div class="tabs is-centered is-small">
            <ul>
              <list-idx-nav v-for="idx in filtered_index" :key="idx" :name="idx" :anchor="idx"></list-idx-nav>
            </ul>
          </div>
        </div>
      </div>
    </div>
  </section>
</template>

<script>
import ListIdxNav from '@/components/ListIdxNav'

export default {
  name: 'IndexList',
  components: { ListIdxNav },

  props: ['index'],

  computed: {
    filtered_index () {
      const specialChars = '!"#$%&\'()*+,-./:;<=>?@[\\]^`{|}~'
      return this.index.filter(c => !specialChars.includes(c))
    }
  }
}
</script>

<style>
</style>
