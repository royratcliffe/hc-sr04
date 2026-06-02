/*
 * SPDX-License-Identifier: MIT
 */

/*!
 * \brief Alternative container-of macro without statement expression.
 * \param ptr Pointer to member.
 * \param type Type of container structure.
 * \param member Name of container's member.
 */
#define containerof(ptr, type, member) ((type *)((char *)(ptr) - offsetof(type, member)))
