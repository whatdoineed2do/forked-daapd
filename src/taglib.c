/* Copyright (C) 2023 Ray whatdoineed2do at nospam gmail com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <tag_c.h>

#include "misc.h"
#include "logger.h"
#include "db.h"


// snyc items back to underlying file
int  taglib_tag(const struct media_file_info* mfi_)
{
    TagLib_File* file;
    bool  changes = false;
    int ret = 0;

    taglib_set_strings_unicode(1);

    if ( (file = taglib_file_new(mfi_->path)) == NULL || (file && !taglib_file_is_valid(file)) )
	return -1;

    char*  value;

    value = safe_asprintf("%d", mfi_->rating);
    taglib_property_set(file, "rating", value);
    free(value);

    if (changes) {
	ret = taglib_file_save(file) ? 0 : -1;
    }
    taglib_file_free(file);

    return ret;
}
