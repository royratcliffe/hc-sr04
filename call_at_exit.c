#include "call_at_exit.h"
#include "cons.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/*!
 * \brief Alternative container-of macro without statement expression.
 * \param ptr Pointer to member.
 * \param type Type of container structure.
 * \param member Name of container's member.
 */
#define containerof(ptr, type, member) ((type *)((char *)(ptr) - offsetof(type, member)))

struct call_at_exit {
  struct cons cell;
  void (*func)(void *);
};

static void set_up(void) __attribute__((constructor));
static void clean_up(void) __attribute__((destructor));

static struct cons *head = CONS_NIL;

void call_at_exit(void (*func)(void *), void *arg1) {
  struct call_at_exit *call = (struct call_at_exit *)malloc(sizeof(struct call_at_exit));
  if (call == NULL) {
    perror("Failed to allocate memory for call_at_exit entry");
    exit(EXIT_FAILURE);
  }
  cons_init(&call->cell, arg1);
  call->func = func;
  (void)cons(&head, &call->cell);
}

void free_at_exit(void *heap) {
  call_at_exit(free, heap);
}

static void set_up(void) { atexit(clean_up); }

static void clean_up(void) {
  struct cons *popped;
  while (CONS_NOT_NIL_P(popped = cons_pop(&head))) {
    struct call_at_exit *call = containerof(popped, struct call_at_exit, cell);
    call->func(cons_car(popped));
    free(call);
  }
}
