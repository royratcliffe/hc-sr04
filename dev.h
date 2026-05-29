#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>

/*!
 * \brief Scan the /dev directory for GPIO chip devices.
 * \param paths Pointer to an array of strings to store the paths of found GPIO chip devices.
 * \return The number of GPIO chip devices found, or -1 on error.
 */
ssize_t scan_dev_for_gpiochips(char ***paths);

/*!
 * \brief Free an array of strings containing paths to GPIO chip devices.
 * \param paths Array of strings to free.
 * \param num_paths Number of paths in the array.
 */
void free_paths(char **paths, ssize_t num_paths);
