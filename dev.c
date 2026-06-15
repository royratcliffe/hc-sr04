#include "dev.h"
#include "dirent_ext.h"

#include <stdlib.h>
#include <sys/stat.h>

#include <gpiod.h>

ssize_t scan_dir_for_gpiochip_paths(const char *dir, char ***paths) {
  /*
   * Filter closure for scandir to find GPIO chip devices. This lambda-like
   * function will be called for each entry in the directory, and it will return
   * 1 if the entry is a GPIO chip device, and 0 otherwise. The function will
   * use gpiod_is_gpiochip_device to check if the entry is a GPIO chip device,
   * and it will also filter out symlinks to avoid false positives. Note that
   * this also means that GPIO chip devices that are symlinks will be ignored,
   * but this is a reasonable trade-off.
   *
   * This is a GNU-compiler extension that allows us to define a function inside
   * another function, and it can capture variables from the enclosing function.
   * In this case, it captures the dir variable from the enclosing
   * scan_dir_for_gpiochip_paths function, which is used to construct the full
   * path of the entry for the gpiod_is_gpiochip_device check. This allows the
   * scan operation to avoid having to pass the dir variable as an argument to
   * the filter function, and it keeps the code more concise and easier to read.
   * However, this is not standard C, and it may not be supported by all
   * compilers, so it should be used with caution if portability is a concern.
   */
  int gpiochip_device_filter(const struct dirent *entry) {
    char *path;
    if (dir_entry_path(dir, entry, &path) < 0) {
      return 0;
    }
    struct stat st;
    if (lstat(path, &st) < 0 || S_ISLNK(st.st_mode)) {
      free(path);
      return 0;
    }
    const int rc = gpiod_is_gpiochip_device(path);
    free(path);
    return rc;
  }
  struct dirent **entries;
  ssize_t num_entries = scandir(dir, &entries, gpiochip_device_filter, NULL);
  if (num_entries < 0) {
    return num_entries;
  }

  /*
   * Create an array of strings to store the paths of found GPIO chip devices.
   * The caller is responsible for freeing this array using free_gpiochip_paths.
   * Note that the number of found GPIO chip devices may be less than the number
   * of entries returned by scandir, because some entries may be filtered out by
   * the gpiochip_device_filter function. The caller should use the returned
   * number of found GPIO chip devices to determine how many paths are in the
   * array. If an error occurs while creating the array of paths, the function
   * will free any allocated memory and return -1.
   */
  char **found = malloc((num_entries + 1) * sizeof(char *));
  if (!found) {
    free_dir_entries(entries, num_entries);
    return -1;
  }
  ssize_t num_found = 0;
  for (ssize_t i = 0; i < num_entries; i++) {
    char *path;
    if (dir_entry_path(dir, entries[i], &path) < 0) {
      free_gpiochip_paths(found, num_found);
      free_dir_entries(entries, num_entries);
      return -1;
    }
    found[num_found++] = path;
  }
  free_dir_entries(entries, num_entries);

  /*
   * Null-terminate the array of found paths. This is not strictly necessary,
   * but it can be useful for debugging and for certain use cases where the
   * caller may want to iterate over the array of paths without knowing the
   * number of paths in advance. The caller should still use the returned number
   * of found GPIO chip devices to determine how many paths are in the array,
   * and should not rely on the null-termination to determine the end of the
   * array. If the caller does not want the null-termination, it can simply
   * ignore the last element of the array.
   */
  found[num_found] = NULL;
  if (paths) {
    *paths = found;
  } else {
    free_gpiochip_paths(found, num_found);
  }
  return num_found;
}

void free_gpiochip_paths(char **paths, ssize_t num_paths) {
  for (ssize_t i = 0; i < num_paths; i++) {
    free(paths[i]);
  }
  free(paths);
}
