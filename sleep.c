/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2026, Roy Ratcliffe, Northumberland, United Kingdom
 */

/*!
 * \file sleep.c
 * \brief Implementation of sleep functions.
 */
#include "sleep.h"

#include <errno.h>
#include <time.h>

int sleep_ns(long ns) {
  struct timespec duration = {.tv_sec = ns / 1000000000, .tv_nsec = ns % 1000000000};
  for (struct timespec rem; nanosleep(&duration, &rem) < 0; duration = rem) {
    if (errno != EINTR) {
      return -1;
    }
  }
  return 0;
}
