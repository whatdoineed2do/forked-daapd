<template>
 <div>
    <content-with-heading>
      <template slot="heading-right">
        <a class="button" :class="{ 'is-info': edit_mode }" @click="start_edit">
          <span class="icon">
            <i class="mdi mdi-content-save"></i>
          </span>
          <span>Save</span>
        </a>
      </template>
      <template slot="content">
        <draggable class="list-group"
           v-model="foo"
           v-bind="dragOptions"
           @start="drag=true"
           @update="update_foo"
           @end="end_foo">
         <transition-group type="transition" :name="!drag ? 'flip-list' : null">
          <list-item-foo v-for="(item, index) in foo"
            :key="item.id" :item="item"
            :current_position=index
            :show_only_next_items=false
            :edit_mode="edit_mode">
              <template slot="actions">
                <a @click="open_dialog(item)" v-if="!edit_mode">
                  <span class="icon has-text-dark"><i class="mdi mdi-dots-vertical mdi-18px"></i></span>
                </a>
                <a @click="remove(item)" v-if="item.id !== state.item_id && edit_mode">
                  <span class="icon has-text-grey"><i class="mdi mdi-delete mdi-18px"></i></span>
                </a>
              </template>
            </list-item-foo>
         </transition-group>
        </draggable>
      </template>
   </content-with-heading>
    <content-with-heading>
      <template slot="content">
        <draggable
           v-model="queue_items"
           v-bind="dragOptions"
           @start="move_start"
           @update="move_item"
           @end="move_end">
         <transition-group type="transition" :name="!drag ? 'flip-list' : null">
          <list-item-queue-item v-for="(item, index) in queue_items"
            :key="item.id" :item="item" :position="index"
            :current_position="current_position"
            :show_only_next_items="show_only_next_items"
            :edit_mode="edit_mode">
              <template slot="actions">
                <a @click="open_dialog(item)" v-if="!edit_mode">
                  <span class="icon has-text-dark"><i class="mdi mdi-dots-vertical mdi-18px"></i></span>
                </a>
                <a @click="remove(item)" v-if="item.id !== state.item_id && edit_mode">
                  <span class="icon has-text-grey"><i class="mdi mdi-delete mdi-18px"></i></span>
                </a>
              </template>
            </list-item-queue-item>
         </transition-group>
        </draggable>
      </template>
   </content-with-heading>
 </div>
</template>

<script>
import draggable from 'vuedraggable'
import ContentWithHeading from '@/templates/ContentWithHeading'
import ListItemQueueItem from '@/components/ListItemQueueItem'
import ListItemFoo from '@/components/ListItemFoo'
import webapi from '@/webapi'

export default {
  name: 'PageFoo',
  display: 'Transitions',
  order: 7,
  components: {
    draggable, ContentWithHeading, ListItemQueueItem, ListItemFoo
  },
  data () {
    return {
      foo: [
        { id: 0, title: 'Hes the Greatest Dancer', artist: 'Sister Sledge', album: 'Sledgehammer' },
        { id: 1, title: 'Lets Get It On', artist: 'Marvin Gaye', album: 'Greatest Hits' },
        { id: 2, title: 'Susan Said', artist: 'David Toa', album: 'Great Leap Forward' }
      ],

      drag: false,
      edit_mode: true,
      show_only_next_items: false
    }
  },
  methods: {
    update_foo: function (e) {
      var oldPosition = !this.show_only_next_items ? e.oldIndex : e.oldIndex
      var newPosition = (e.newIndex - e.oldIndex)
      console.log('update_foo:  old=' + oldPosition + ' new=' + newPosition)
    },
    end_foo: function (e) {
      var oldPosition = !this.show_only_next_items ? e.oldIndex : e.oldIndex
      var newPosition = e.newIndex - e.oldIndex
      console.log('end_foo:  old=' + oldPosition + ' new=' + newPosition)
    },

    move_start: function (e) {
      this.drag = true
      console.log('move_start: old=' + e.oldIndex + ' new=' + e.newIdex)
    },
    move_end: function (e) {
      this.drag = false
      console.log('move_end: old=' + e.oldIndex + ' new=' + e.newIdex)
    },
    move_item: function (e) {
      var oldPosition = !this.show_only_next_items ? e.oldIndex : e.oldIndex + this.current_position
      var item = this.queue_items[oldPosition]
      var newPosition = item.position + (e.newIndex - e.oldIndex)
      if (newPosition !== oldPosition) {
        webapi.queue_move(item.id, newPosition)
      }
      console.log('move_item: old=' + oldPosition + ' new=' + newPosition)
    },

    start_edit: function () {
      if (!this.edit_mode) {
        this.edit_mode = true
      } else {
        // webapi.queue_move(item.id, newPosition)
        this.edit_mode = false
      }
    }
  },

  computed: {
    state () {
      return this.$store.state.player
    },
    queue () {
      return this.$store.state.queue
    },
    queue_items: {
      get () { return this.$store.state.queue.items },
      set (value) {
        // this.$store.state.queue.items = value
        // console.log(JSON.stringify(value, null, 2))
        console.log('queue set')
      }
    },
    current_position () {
      const nowPlaying = this.$store.getters.now_playing
      return nowPlaying === undefined || nowPlaying.position === undefined ? -1 : this.$store.getters.now_playing.position
    },
    foo_compute: {
      get () { return this.foo },
      set (value) { console.log(JSON.stringify(value, null, 2)) }
    },
    dragOptions () {
      return {
        animation: 200,
        group: 'description',
        disabled: false,
        ghostClass: 'ghost'
      }
    }
  }
}
</script>

<style>
.flip-list-move {
  transition: transform 0.2s;
}

.no-move {
  transition: transform 0s;
}

.ghost {
  opacity: 1;
  background: #c8ebfb;
}

.list-group {
  min-height: 20px;
}

.list-group-item {
  cursor: move;
}

.list-group-item i {
  cursor: pointer;
}
</style>
