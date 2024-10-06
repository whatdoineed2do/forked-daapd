const GROUP_KEY_NONE = 'GROUP_KEY_NONE'

export function noop() {
  return {
    compareFn: null,
    groupKeyFn: (item) => GROUP_KEY_NONE
  }
}

/*
 * Keep default sorting of item list and build index and group by given field
 */
export function bySortName(field, defaultValue = '_') {
  return {
    // Keep the sort order of the original item list
    // Assumes that the list is already ordered by name
    compareFn: null,

    groupKeyFn: (item) => {
      const fieldValue = item[field] || defaultValue
      return fieldValue.charAt(0).toUpperCase()
    }
  }
}

export function byName(field, defaultValue = '_') {
  return {
    compareFn: (a, b) => {
      const fieldA = a[field] || defaultValue
      const fieldB = b[field] || defaultValue

      return fieldA.localeCompare(fieldB)
    },

    groupKeyFn: (item) => {
      const fieldValue = item[field] || defaultValue
      return fieldValue.charAt(0).toUpperCase()
    }
  }
}

export function byYear(field, { direction = 'asc', defaultValue = '0000' }) {
  return {
    compareFn: (a, b) => {
      const fieldA = a[field] || defaultValue
      const fieldB = b[field] || defaultValue

      const result = fieldA.localeCompare(fieldB)
      return direction === 'asc' ? result : result * -1
    },

    groupKeyFn: (item) => {
      const fieldValue = item[field] || defaultValue
      return fieldValue.substring(0, 4)
    }
  }
}

export function byDateSinceToday(field, defaultValue = '0000') {
  return {
    compareFn: (a, b) => {
      const fieldA = a[field] || defaultValue
      const fieldB = b[field] || defaultValue

      return fieldB.localeCompare(fieldA)
    },

    groupKeyFn: (item) => {
      const fieldValue = item[field]

      if (!fieldValue) {
        return defaultValue
      }

      const diff = new Date().getTime() - new Date(fieldValue).getTime()

      if (diff < 86400000) {
        // 24h
        return 'Today'
      } else if (diff < 604800000) {
        // 7 days
        return 'Last week'
      } else if (diff < 2592000000) {
        // 30 days
        return 'Last month'
      } else if (diff < 7776000000) {
        // 90 days
        return 'Last quarter'
      }
      return fieldValue.substring(0, 4)
    }
  }
}

export function byRating({ direction = 'desc', defaultValue = 0 }) {
  return {
    compareFn: (a, b) => {
      const fieldA = a['rating'] || defaultValue
      const fieldB = b['rating'] || defaultValue

      const result = fieldA < fieldB ? -1 : fieldA > fieldB ? 1 : 0
      return direction === 'asc' ? result : result * -1
    },

    groupKeyFn: (item) => {
      const fieldValue = item['rating'] || defaultValue
      return Math.floor(fieldValue / 10)
    }
  }
}

export function byMedium(field, direction = 'asc', defaultValue = 1) {
  return {
    compareFn: (a, b) => {
      const fieldA = a[field] || defaultValue
      const fieldB = b[field] || defaultValue
      const result = fieldA - fieldB
      return direction === 'asc' ? result : result * -1
    },

    groupKeyFn: (item) => {
      return item[field] || defaultValue
    }
  }
}

function basename(path) {
  return path.slice(path.lastIndexOf('/') + 1, path.length)
}

export function byPath({ direction = 'asc', defaultValue = '_' }) {
  return {
    compareFn: (a, b) => {
      let fieldA = a['path'] || defaultValue
      let fieldB = b['path'] || defaultValue

      fieldA = basename(fieldA)
      fieldB = basename(fieldB)

      const result = fieldA < fieldB ? -1 : fieldA > fieldB ? 1 : 0
      return direction === 'asc' ? result : result * -1
    },

    groupKeyFn: (item) => {
      let fieldValue = item['path'] || defaultValue
      fieldValue = basename(fieldValue)
      return fieldValue.charAt(0).toUpperCase()
    }
  }
}


export class GroupByList {
  constructor({ items = [], total = 0, offset = 0, limit = -1 } = {}) {
    this.items = items
    this.total = total
    this.offset = offset
    this.limit = limit

    this.count = items.length
    this.indexList = []
    this.group(noop())
  }

  get() {
    return this.itemsByGroup
  }

  isEmpty() {
    return !this.items || this.items.length <= 0
  }

  group(options, filterFns = []) {
    const itemsFiltered = filterFns
      ? this.items.filter((item) => filterFns.every((fn) => fn(item)))
      : this.items
    this.count = itemsFiltered.length

    // Sort item list
    let itemsSorted = options.compareFn
      ? [...itemsFiltered].sort(options.compareFn)
      : itemsFiltered

    // Create index list
    this.indexList = [...new Set(itemsSorted.map(options.groupKeyFn))]

    // Group item list
    this.itemsByGroup = itemsSorted.reduce((r, item) => {
      const groupKey = options.groupKeyFn(item)
      r[groupKey] = [...(r[groupKey] || []), item]
      return r
    }, {})
  }

  [Symbol.iterator]() {
    // Use a new index for each iterator. This makes multiple
    // iterations over the iterable safe for non-trivial cases,
    // such as use of break or nested looping over the same iterable.
    let groupIndex = -1
    let itemIndex = -1

    return {
      next: () => {
        /*
        console.log(
          '[group-by-list] itemIndex=' +
            itemIndex +
            ', groupIndex=' +
            groupIndex
        )
         */
        if (groupIndex >= this.indexList.length) {
          // We reached the end of all groups and items
          //
          // This should never happen, as the we already
          // return "done" after we reached the last item
          // of the last group
          /*
            console.log(
            '[group-by-list] done! (groupIndex >= this.indexList.length)'
          )
           */
          return { done: true }
        } else if (groupIndex < 0) {
          // We start iterating
          //
          // Return the first group title as the next item
          ++groupIndex
          itemIndex = 0

          if (this.indexList[groupIndex] !== GROUP_KEY_NONE) {
            // Only return the group, if it is not the "noop" default group
            return {
              value: {
                groupKey: this.indexList[groupIndex],
                itemId: this.indexList[groupIndex],
                isItem: false,
                item: {}
              },
              done: false
            }
          }
        }

        let currentGroupKey = this.indexList[groupIndex]
        let currentGroupItems = this.itemsByGroup[currentGroupKey]

        if (itemIndex < currentGroupItems.length) {
          // We are in a group with items left
          //
          // Return the current item and increment the item index
          const currentItem = this.itemsByGroup[currentGroupKey][itemIndex++]
          return {
            value: {
              groupKey: currentGroupKey,
              itemId: currentItem.id,
              isItem: true,
              item: currentItem
            },
            done: false
          }
        } else {
          // We reached the end of the current groups item list
          //
          // Move to the next group and return the group key/title
          // as the next item
          ++groupIndex
          itemIndex = 0

          if (groupIndex < this.indexList.length) {
            currentGroupKey = this.indexList[groupIndex]
            return {
              value: {
                groupKey: currentGroupKey,
                itemId: currentGroupKey,
                isItem: false,
                item: {}
              },
              done: false
            }
          } else {
            // No group left, we are done iterating
            /*
            console.log(
              '[group-by-list] done! (groupIndex >= this.indexList.length)'
            )
             */
            return { done: true }
          }
        }
      }
    }
  }
}
