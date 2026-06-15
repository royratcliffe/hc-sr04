/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2026, Roy Ratcliffe, Northumberland, United Kingdom
 */

/*!
 * \file dirent_ext.h
 * \brief Header file for extended dirent functions.
 */
#ifndef DIRENT_EXT_H
#define DIRENT_EXT_H

#include <dirent.h>
#include <sys/types.h>

/*!
 * \brief Construct a path string from a directory and a dirent entry.
 * \param dir The directory path.
 * \param entry The dirent entry.
 * \param path Pointer to a string to store the constructed path.
 * \return 0 on success, or -1 on error.
 */
int dir_entry_path(const char *dir, const struct dirent *entry, char **path);

/*!
 * \brief Free an array of dirent entries.
 * \param entries Array of dirent entries to free.
 * \param num_entries Number of entries in the array.
 */
void free_dir_entries(struct dirent **entries, ssize_t num_entries);

#endif /* DIRENT_EXT_H */
