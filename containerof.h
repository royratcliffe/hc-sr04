/*
 * SPDX-License-Identifier: MIT
 */

/*!
 * \file containerof.h
 * \brief Alternative container-of macro without statement expression.
 * \details This header defines a containerof macro that can be used in C11
 * without relying on GNU extensions. The macro calculates the address of the
 * containing structure given a pointer to one of its members, the type of the
 * container structure, and the name of the member. This is useful for
 * implementing data structures and callbacks where you have a pointer to a
 * member and need to access the containing structure. The macro uses the
 * standard offsetof macro to compute the offset of the member within the
 * structure and subtracts that offset from the pointer to the member to get the
 * pointer to the container structure.
 */
#ifndef CONTAINEROF_H
#define CONTAINEROF_H

#include <stddef.h>

/*!
 * \brief Alternative container-of macro without statement expression.
 * \param ptr Pointer to member.
 * \param type Type of container structure.
 * \param member Name of container's member.
 */
#define containerof(ptr, type, member) ((type *)((char *)(ptr) - offsetof(type, member)))

#endif /* CONTAINEROF_H */
