#include "dirent_ext.h"

#include <stdlib.h>

/*
 * for asprintf
 */
#include <stdio.h>

int dir_entry_path(const char *dir, const struct dirent *entry, char **path) { return asprintf(path, "%s/%s", dir, entry->d_name); }

void free_dir_entries(struct dirent **entries, ssize_t num_entries) {
  for (ssize_t i = 0; i < num_entries; i++) {
    free(entries[i]);
  }
  free(entries);
}
