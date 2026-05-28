#include <stdlib.h>
#include <dirent.h>

int dev_entry_path(const struct dirent *entry, char **path);
int gpiochip_device_filter(const struct dirent *entry);
void free_entries(struct dirent **entries, ssize_t num_entries);
void free_paths(char **paths, ssize_t num_paths);
ssize_t scan_dev_for_gpiochips(char ***paths);
