/* SPDX-License-Identifier: MIT */

/*!
 * \file cons.c
 * \brief Implementation of cons cell operations.
 * \details This file implements functions for manipulating cons cells,
 * which are fundamental data structures in Lisp-like languages. The
 * functions link caller-provided cons cells into lists, remove cells
 * from a list, and reverse a list of cons cells. The operations work
 * with the structure defined in \c cons.h to link and manipulate linked
 * lists and other complex data structures.
 */
#include <cons.h>

/*
 * for malloc and free
 */
#include <stdlib.h>

static bool cons_delete_p(struct cons **list, struct cons *cell, void *user);

struct cons **cons(struct cons **list, struct cons *cell) {
  cons_rplacd(cell, *list);
  *list = cell;
  return &cell->cdr;
}

struct cons **cons_prepend(struct cons **list, struct cons *cell) {
  (void)cons(list, cell);
  return list;
}

struct cons **cons_loop(struct cons **list, bool (*pred)(struct cons **list, struct cons *cell, void *user),
                        void *user) {
  for (struct cons *cell = *list; CONS_NOT_NIL_P(cell); list = &cell->cdr, cell = cons_cdr(cell))
    if (pred(list, cell, user)) return list;
  /*
   * Returning NULL is correct. It is a NULL pointer to a pointer to a
   * cons cell---a pointer to the head of a list. Therefore not itself a
   * cons cell pointer. Returning NULL indicates that the predicate did
   * not find a matching cell in the list, and thus there is no pointer
   * to a cons cell that satisfies the predicate.
   *
   * This design allows the function to signal the absence of a matching
   * cell without returning a pointer to a cons cell, which would be
   * misleading since it would suggest that a valid cell was found when
   * in fact it was not. By returning NULL, the function provides a
   * unambiguous way to indicate that the search was unsuccessful.
   */
  return NULL;
}

static bool cons_find_p(struct cons **list, struct cons *cell, void *user) {
  (void)list;
  return cell == user;
}

struct cons **cons_find(struct cons **list, void *cell) { return cons_loop(list, cons_find_p, cell); }

static bool cons_delete_p(struct cons **list, struct cons *cell, void *user) {
  (void)list;
  return cons_car(cell) == user;
}

struct cons *cons_delete(struct cons **list, void *car) {
  struct cons **found = cons_loop(list, cons_delete_p, car);
  if (found == NULL) return CONS_NIL;
  struct cons *deleted = *found;
  *found = cons_cdr(deleted);
  return deleted;
}

struct cons *cons_remove(struct cons **list, struct cons *cell) {
  struct cons **found = cons_find(list, cell);
  if (found == NULL) return CONS_NIL;
  struct cons *removed = *found;
  *found = cons_cdr(removed);
  return removed;
}

struct cons *cons_pop(struct cons **list) {
  struct cons *cell = *list;
  if (CONS_NIL_P(cell)) return CONS_NIL;
  *list = cons_cdr(cell);
  return cell;
}

void cons_reverse(struct cons **list) {
  /*
   * Maintain a pointer to the reversed list (initially CONS_NIL) and
   * iteratively traverse the original list. For each cell, save the next cell
   * (cdr) before reassigning the cdr of the current cell to point to the
   * reversed list. Then update the reversed list to be the current cell and
   * move to the next cell in the original list. This process continues until
   * reversing has traversed the entire original list, at which point the
   * reversed list will contain all the cells in reverse order.
   */
  struct cons *reversed = CONS_NIL, *cell = *list;
  while (CONS_NOT_NIL_P(cell)) {
    struct cons *tail = cons_cdr(cell);
    cons_rplacd(cell, reversed);
    reversed = cell;
    cell = tail;
  }
  *list = reversed;
}

size_t cons_length(const struct cons *cell) {
  size_t length = 0;
  while (CONS_NOT_NIL_P(cell)) {
    length++;
    cell = cons_cdr(cell);
  }
  return length;
}

struct cons *cons_last(struct cons *cell) {
  if (CONS_NIL_P(cell)) return CONS_NIL;
  /*
   * The last cell in a list is the one having its cdr field equal to CONS_NIL.
   */
  struct cons *tail;
  while (CONS_NOT_NIL_P(tail = cons_cdr(cell)))
    cell = tail;
  return cell;
}

struct cons *cons_nth(struct cons *cell, size_t nth) {
  while (CONS_NOT_NIL_P(cell) && nth != 0) {
    cell = cons_cdr(cell);
    nth--;
  }
  return cell;
}

struct cons *cons_member(struct cons *cell, void *car) {
  for (; CONS_NOT_NIL_P(cell); cell = cons_cdr(cell))
    if (cons_car(cell) == car) return cell;
  return CONS_NIL;
}

struct cons *cons_append(struct cons *cell1, struct cons *cell2) {
  struct cons *last = cons_last(cell1);
  if (CONS_NIL_P(last)) return cell2;
  cons_rplacd(last, cell2);
  return cell1;
}

struct cons *cons_heap(void *car) {
  struct cons *cell = (struct cons *)malloc(sizeof(struct cons));
  if (CONS_NOT_NIL_P(cell)) cons_init(cell, car);
  return cell;
}

void cons_free(struct cons *cell) {
  /*
   * Check for CONS_NIL before freeing the cell, as CONS_NIL is a
   * special value that represents "the" empty list and should not be
   * freed. If the cell is not CONS_NIL, assume it is safe to free the
   * heap-allocated memory for that cell. This check prevents accidental
   * deallocation of the CONS_NIL sentinel value; although, strictly
   * speaking, the implementation can free(NULL) without causing
   * undefined behavior.
   */
  if (CONS_NOT_NIL_P(cell)) free(cell);
}
