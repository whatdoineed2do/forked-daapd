
export default class Tracks {
  constructor (items, options = { hideSingles: false, hideSpotify: false, sort: 'Name', min_rating: 0, group: false }) {
    this.items = items
    this.options = options
    this.grouped = {}
    this.sortedAndFiltered = []
    this.indexList = []

    this.init()
  }

  init () {
    this.createSortedAndFilteredList()
    this.createGroupedList()
    this.createIndexList()
  }

  getTrackIndex (track) {
    if (this.options.sort === 'Recently added') {
      return track.time_added.substring(0, 4)
    } else if (this.options.sort === 'Recently added (browse)') {
      return this.getRecentlyAddedBrowseIndex(track.time_added)
    } else if (this.options.sort === 'Recently released') {
      return track.date_released ? track.date_released.substring(0, 4) : '0000'
    } else if (this.options.sort === 'Release date') {
      return track.date_released ? track.date_released.substring(0, 4) : '0000'
    }
    return track.title_sort.charAt(0).toUpperCase()
  }

  getRecentlyAddedBrowseIndex (recentlyAdded) {
    if (!recentlyAdded) {
      return '0000'
    }

    const diff = new Date().getTime() - new Date(recentlyAdded).getTime()

    if (diff < 86400000) { // 24h
      return 'Today'
    } else if (diff < 604800000) { // 7 days
      return 'Last week'
    } else if (diff < 2592000000) { // 30 days
      return 'Last month'
    }
    return recentlyAdded.substring(0, 4)
  }

  createIndexList () {
    this.indexList = [...new Set(this.sortedAndFiltered
      .map(track => this.getTrackIndex(track)))]
  }

  createSortedAndFilteredList () {
    let tracksSorted = this.items
    if (this.options.sort === 'Recently added' || this.options.sort === 'Recently added (browse)') {
      tracksSorted = [...tracksSorted].sort((a, b) => b.time_added.localeCompare(a.time_added))
    } else if (this.options.sort === 'Recently released') {
      tracksSorted = [...tracksSorted].sort((a, b) => {
        if (!a.date_released) {
          return 1
        }
        if (!b.date_released) {
          return -1
        }
        return b.date_released.localeCompare(a.date_released)
      })
    } else if (this.options.sort === 'Release date') {
      tracksSorted = [...tracksSorted].sort((a, b) => {
        if (!a.date_released) {
          return -1
        }
        if (!b.date_released) {
          return 1
        }
        return a.date_released.localeCompare(b.date_released)
      })
    }
    this.sortedAndFiltered = tracksSorted.filter(track => track.rating >= this.options.min_rating)
  }

  createGroupedList () {
    if (!this.options.group) {
      this.grouped = {}
    }
    this.grouped = this.sortedAndFiltered.reduce((r, track) => {
      const idx = this.getTrackIndex(track)
      r[idx] = [...r[idx] || [], track]
      return r
    }, {})
  }
}
