#include "call_at.h"
#include "cons.h"
#include "containerof.h"
#include "pr.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

struct call_at {
  struct cons cell;
  void (*func)(void *);
};

void call_at(struct cons **ca, void (*func)(void *), void *arg1) {
  struct call_at *call_at = malloc(sizeof(struct call_at));
  if (call_at == NULL) {
    pr_err("Failed to allocate memory for call_at entry");
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
