<template>
  <div
    v-if="$route.query.directory"
    class="media"
    @click="open_parent_directory()"
  >
    <figure class="media-left fd-has-action">
      <span class="icon">
        <mdicon name="subdirectory-arrow-left" size="16" />
      </span>
    </figure>
    <div class="media-content fd-has-action is-clipped">
      <h1 class="title is-6">..</h1>
    </div>
    <div class="media-right">
      <slot name="actions" />
    </div>
  </div>
  <template v-for="directory in directories" :key="directory.item.path">
   <div v-if="!directory.isItem && !hide_group_title" class="mt-6 mb-5 py-2">
      <span
        :id="'index_' + groupKey(directory)"
        class="tag is-info is-light is-small has-text-weight-bold"
        >{{ directory.groupKey }}</span
      >
    </div>
    <div v-else-if="directory.isItem" class="media" @click="open_directory(directory.item)">
      <figure class="media-left fd-has-action">
        <span class="icon">
          <mdicon name="folder" size="16" />
        </span>
      </figure>
      <div class="media-content fd-has-action is-clipped">
	<div style="margin-top: 0.7rem">
	  <h1 class="title is-6">
	    {{ directory.item.path.substring(directory.item.path.lastIndexOf('/') + 1) }}
	  </h1>
	  <h2 class="subtitle is-7 has-text-grey-light">
	    {{ directory.item.path }}
	  </h2>
	</div>
      </div>
      <div class="media-right" style="padding-top: 0.7rem">
        <a @click.prevent.stop="open_dialog(directory.item)">
          <span class="icon has-text-dark"
            ><mdicon name="dots-vertical" size="16"
          /></span>
        </a>
      </div>
    </div>
  </template>
  <teleport to="#app">
    <modal-dialog-directory
      :show="show_details_modal"
      :directory="selected_directory"
      @close="show_details_modal = false"
    />
  </teleport>
</template>

<script>
import ModalDialogDirectory from '@/components/ModalDialogDirectory.vue'

export default {
  name: 'ListDirectories',
  components: { ModalDialogDirectory },

  props: ['directories'],

  data() {
    return {
      show_details_modal: false,
      selected_directory: {}
    }
  },

  computed: {
    current_directory() {
      if (this.$route.query && this.$route.query.directory) {
        return this.$route.query.directory
      }
      return '/'
    }
  },

  methods: {
    open_parent_directory: function () {
      const parent = this.current_directory.slice(
        0,
        this.current_directory.lastIndexOf('/')
      )
      if (
        parent === '' ||
        this.$store.state.config.directories.includes(this.current_directory)
      ) {
        this.$router.push({ path: '/files' })
      } else {
        this.$router.push({
          path: '/files',
          query: {
            directory: this.current_directory.slice(
              0,
              this.current_directory.lastIndexOf('/')
            )
          }
        })
      }
    },

    open_directory: function (directory) {
      this.$router.push({
        path: '/files',
        query: { directory: directory.path }
      })
    },

    groupKey: function (where) {
      if (where.groupKey) return where.groupKey.replace(' ', '')

      return ''
    },

    open_dialog: function (directory) {
      this.selected_directory = directory
      this.show_details_modal = true
    }
  }
}
</script>

<style></style>
