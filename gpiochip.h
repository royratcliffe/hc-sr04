#include <sys/types.h>

/*!
 * \brief Scan a directory for GPIO chip devices.
 * \param dir The directory to scan.
 * \param paths Pointer to an array of strings to store the paths of found GPIO chip devices.
 * \return The number of GPIO chip devices found, or -1 on error.
 */
ssize_t scan_dir_for_gpiochip_paths(const char *dir, char ***paths);

/*!
 * \brief Free an array of strings containing paths to GPIO chip devices.
 * \param paths Array of strings to free.
 * \param num_paths Number of paths in the array.
 */
void free_gpiochip_paths(char **paths, ssize_t num_paths);
