<template>
  <div class="fd-page-with-tabs">
    <tabs-settings />

    <content-with-heading>
      <template #heading-left>
        <div class="title is-4">Queue Adding</div>
      </template>

      <template #content>
        <div class="content">
          <p>
            Tracks added to the queue can be duplicate of existing entries - we can request the server to 'replace' existing tracks with newly queued items so they are not repeated
          </p>
        </div>
        <settings-checkbox category_name="custom" option_name="queue_add_replace_tracks">
          <template #label> Replace existing on add</template>
        </settings-checkbox>
      </template>
    </content-with-heading>
  </div>
</template>

<script>
import ContentWithHeading from '@/templates/ContentWithHeading.vue'
import TabsSettings from '@/components/TabsSettings.vue'
import SettingsCheckbox from '@/components/SettingsCheckbox.vue'

export default {
  name: 'SettingsPageCustom',
  components: { ContentWithHeading, TabsSettings, SettingsCheckbox }
}
</script>

<style>
</style>
