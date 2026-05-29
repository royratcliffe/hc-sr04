#include "cons.h"

#include <stdio.h>
#include <stdlib.h>

static void set_up(void) __attribute__((constructor));
static void clean_up(void);

static struct cons *head = CONS_NIL;

void free_at_exit(void *car) {
  struct cons *cell = cons_heap(car);
  if (CONS_NIL_P(cell)) {
    perror("Failed to allocate memory for clean-up cons cell");
    exit(EXIT_FAILURE);
  }
  (void)cons(&head, cell);
}

static void set_up(void) { atexit(clean_up); }

static void clean_up(void) {
  while (CONS_NOT_NIL_P(head)) {
    struct cons *cell = head;
    head = cons_cdr(cell);
    free(cons_car(cell));
    cons_free(cell);
  }
}
