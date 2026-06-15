#include "dev.h"

#include <stdio.h>
#include <sys/stat.h>

#include <gpiod.h>

const char *const dev = "/dev";

static int dev_entry_path(const struct dirent *entry, char **path);
static int gpiochip_device_filter(const struct dirent *entry);
static void free_entries(struct dirent **entries, ssize_t num_entries);

ssize_t scan_dev_for_gpiochips(char ***paths) {
  struct dirent **entries;
  int num_entries = scandir(dev, &entries, gpiochip_device_filter, NULL);
  if (num_entries < 0) {
    return num_entries;
  }
  char **found = malloc(num_entries * sizeof(char *));
  if (!found) {
    free_entries(entries, num_entries);
    return -1;
  }
  int num_found = 0;
  for (int i = 0; i < num_entries; i++) {
    char *path;
    if (dev_entry_path(entries[i], &path) < 0) {
      free_paths(found, num_found);
      free_entries(entries, num_entries);
      return -1;
    }
    found[num_found++] = path;
  }
  free_entries(entries, num_entries);
  *paths = found;
  return num_found;
}

int dev_entry_path(const struct dirent *entry, char **path) { return asprintf(path, "%s/%s", dev, entry->d_name); }

int gpiochip_device_filter(const struct dirent *entry) {
  char *path;
  if (dev_entry_path(entry, &path) < 0) {
    return 0;
  }
  /*
   * Filter out symlinks to avoid false positives. Note that this also means
   * that GPIO chip devices that are symlinks will be ignored, but this is a
   * reasonable trade-off.
   */
  struct stat st;
  if (lstat(path, &st) < 0 || S_ISLNK(st.st_mode)) {
    free(path);
    return 0;
  }
  const int rc = gpiod_is_gpiochip_device(path);
  free(path);
  return rc;
}

void free_entries(struct dirent **entries, ssize_t num_entries) {
  for (ssize_t i = 0; i < num_entries; i++) {
    free(entries[i]);
  }
  free(entries);
}

void free_paths(char **paths, ssize_t num_paths) {
  for (ssize_t i = 0; i < num_paths; i++) {
    free(paths[i]);
  }
  free(paths);
}
