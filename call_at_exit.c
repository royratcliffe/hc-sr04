/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2026, Roy Ratcliffe, Northumberland, United Kingdom
 */

/*!
 * \file call_at_exit.c
 * \brief Implementation of scheduling and calling functions to be executed at program exit.
 */
#include "call_at_exit.h"
#include "call_at.h"
#include "cons.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

static void set_up(void) __attribute__((constructor));
static void clean_up(void);

static struct cons *at_exit = CONS_NIL;

void call_at_exit(void (*func)(void *), void *arg1) { call_at(&at_exit, func, arg1); }

void free_at_exit(void *heap) { call_at_exit(free, heap); }

static void set_up(void) { atexit(clean_up); }

static void clean_up(void) { call_up(&at_exit); }
