#include "dev.h"
#include "dirent_ext.h"

#include <stdlib.h>
#include <sys/stat.h>

#include <gpiod.h>

const char *const dev = "/dev";

static int gpiochip_device_filter(const struct dirent *entry);

ssize_t scan_dev_for_gpiochips(char ***paths) {
  struct dirent **entries;
  ssize_t num_entries = scandir(dev, &entries, gpiochip_device_filter, NULL);
  if (num_entries < 0) {
    return num_entries;
  }
  char **found = malloc(num_entries * sizeof(char *));
  if (!found) {
    free_dir_entries(entries, num_entries);
    return -1;
  }
  ssize_t num_found = 0;
  for (ssize_t i = 0; i < num_entries; i++) {
    char *path;
    if (dir_entry_path(dev, entries[i], &path) < 0) {
      free_paths(found, num_found);
      free_dir_entries(entries, num_entries);
      return -1;
    }
    found[num_found++] = path;
  }
  free_dir_entries(entries, num_entries);
  *paths = found;
  return num_found;
}

int gpiochip_device_filter(const struct dirent *entry) {
  char *path;
  if (dir_entry_path(dev, entry, &path) < 0) {
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

void free_paths(char **paths, ssize_t num_paths) {
  for (ssize_t i = 0; i < num_paths; i++) {
    free(paths[i]);
  }
  free(paths);
}
