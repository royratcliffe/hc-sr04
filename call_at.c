/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2026, Roy Ratcliffe, Northumberland, United Kingdom
 */

/*!
 * \file call_at.c
 * \brief Implementation of scheduling and calling functions.
 */
#include "call_at.h"
#include "cons.h"
#include "containerof.h"
#include "pr.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/*!
 * \brief Structure representing a scheduled function call.
 *
 * \details This structure is used to store information about a function that is
 * scheduled to be called later. It contains a cons cell for storing the
 * argument to be passed to the function, and a function pointer to the function
 * itself.
 *
 * \note The cons cell's \c car field holds the argument to be passed to the
 * function, while the \c cdr field is used to link multiple scheduled function
 * calls together in a list.
 */
struct call_at {
  struct cons cell;     /*!< Cell storing the argument to be passed to the function. */
  void (*func)(void *); /*!< Pointer to the function to be called. */
};

void call_at(struct cons **ca, void (*func)(void *), void *arg1) {
  struct call_at *call_at = malloc(sizeof(struct call_at));
  if (call_at == NULL) {
    pr_err("Failed to allocate memory for call_at entry\n");
    exit(EXIT_FAILURE);
  }
  cons_init(&call_at->cell, arg1);
  call_at->func = func;
  (void)cons(ca, &call_at->cell);
}

void call_up(struct cons **ca) {
  struct cons *popped;
  while (CONS_NOT_NIL_P(popped = cons_pop(ca))) {
    struct call_at *call_at = containerof(popped, struct call_at, cell);
    call_at->func(cons_car(popped));
    free(call_at);
  }
}
